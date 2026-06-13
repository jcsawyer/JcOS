#include "main.hpp"

#include "board_config.hpp"
#include "boot_display.hpp"
#include "gpio.hpp"
#include "lcd.hpp"
#include "timer.hpp"
#include "uart.hpp"

#include <arch/cpu.hpp>
#include <stdint.h>
#include <stdio/printf.h>

namespace {

Chainloader::Uart *g_uart = nullptr;

extern "C" uint8_t chainloader_relocator_start;
extern "C" uint8_t chainloader_relocator_end;

using RelocatorFn = void (*)(void *destination, const void *source, size_t size,
                             uintptr_t entryAddress);

enum class LoadError {
  None,
  ShortSize,
  InvalidSize,
  ShortPayload,
  PayloadTooSmall,
  ImageEmpty,
};

struct LoadAttempt {
  LoadError error;
  uint32_t payloadSize;
};

void runLcdSelfTest(Chainloader::LCD &lcd) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.writeString("1234567890ABCDEF");
  lcd.setCursor(1, 0);
  lcd.writeString("GHIJKLMNOPQRSTUV");
  Chainloader::Timer::delayMillis(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.writeString("LCD SELF TEST OK");
  lcd.setCursor(1, 0);
  lcd.writeString("BOOTING...");
  Chainloader::Timer::delayMillis(1500);
}

void copyBytes(void *destination, const void *source, size_t size) {
  auto *dst = static_cast<uint8_t *>(destination);
  auto *src = static_cast<const uint8_t *>(source);
  for (size_t i = 0; i < size; ++i) {
    dst[i] = src[i];
  }
}

void uartLog(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vprintf_(format, args);
  va_end(args);
}

[[noreturn]] void fatal(Chainloader::BootDisplay &display, const char *code,
                        const char *detail, const char *uartMessage) {
  display.showError(code, detail);
  uartLog("[chainloader] ERROR %s: %s\n", code, uartMessage);
  CPU::waitForever();
}

void showRetry(Chainloader::BootDisplay &display, const char *code,
               const char *detail, const char *uartMessage) {
  display.showStage("RETRY", code);
  uartLog("[chainloader] RETRY %s: %s\n", code, uartMessage);
  if (detail != nullptr && detail[0] != '\0') {
    display.showError(code, detail);
  }
  Chainloader::Timer::delayMillis(Chainloader::Board::RETRY_NOTICE_DELAY_MS);
}

bool readByteWithTimeout(Chainloader::Uart &uart, char &value,
                         uint64_t timeoutMicros) {
  const uint64_t startTicks = Chainloader::Timer::ticks();
  while (!Chainloader::Timer::expired(startTicks, timeoutMicros)) {
    if (uart.readByte(value)) {
      return true;
    }
    CPU::nop();
  }
  return false;
}

LoadAttempt waitForPayloadSize(Chainloader::Uart &uart,
                               Chainloader::BootDisplay &display) {
  uartLog("[chainloader] Waiting for host handshake\n");
  display.showStage("WAIT", "HOST");

  char sizeBytes[4] = {};

  while (true) {
    uart.writeByte(0x03);
    uart.writeByte(0x03);
    uart.writeByte(0x03);

    if (!readByteWithTimeout(uart, sizeBytes[0],
                             Chainloader::Board::HOST_REQUEST_TIMEOUT_US)) {
      Chainloader::Timer::delayMicros(
          Chainloader::Board::HOST_RETRY_DELAY_US);
      continue;
    }

    for (size_t i = 1; i < sizeof(sizeBytes); ++i) {
      if (!readByteWithTimeout(uart, sizeBytes[i],
                               Chainloader::Board::PAYLOAD_IO_TIMEOUT_US)) {
        return {LoadError::ShortSize, 0};
      }
    }

    const uint32_t size = static_cast<uint32_t>(
        static_cast<uint8_t>(sizeBytes[0]) |
        (static_cast<uint32_t>(static_cast<uint8_t>(sizeBytes[1])) << 8) |
        (static_cast<uint32_t>(static_cast<uint8_t>(sizeBytes[2])) << 16) |
        (static_cast<uint32_t>(static_cast<uint8_t>(sizeBytes[3])) << 24));

    if (size == 0 || size > Chainloader::Board::MAX_PAYLOAD_SIZE) {
      return {LoadError::InvalidSize, 0};
    }

    uart.writeByte('O');
    uart.writeByte('K');
    uartLog("[chainloader] Host requested %u bytes\n",
            static_cast<unsigned int>(size));
    return {LoadError::None, size};
  }
}

LoadError receivePayload(Chainloader::Uart &uart,
                         Chainloader::BootDisplay &display, uint8_t *destination,
                         uint32_t payloadSize) {
  display.showProgress(0, payloadSize);
  uartLog("[chainloader] Receiving payload into 0x%08X\n",
          static_cast<unsigned int>(
              Chainloader::Board::PAYLOAD_SCRATCH_ADDRESS));

  size_t received = 0;
  size_t lastReported = 0;

  while (received < payloadSize) {
    char byte = 0;
    if (!readByteWithTimeout(uart, byte,
                             Chainloader::Board::PAYLOAD_IO_TIMEOUT_US)) {
      return LoadError::ShortPayload;
    }

    destination[received++] = static_cast<uint8_t>(byte);

    if (received == payloadSize || received - lastReported >= 4096) {
      display.showProgress(received, payloadSize);
      uartLog("[chainloader] RX %u/%u\n", static_cast<unsigned int>(received),
              static_cast<unsigned int>(payloadSize));
      lastReported = received;
    }
  }

  return LoadError::None;
}

LoadError verifyPayload(Chainloader::BootDisplay &display, const uint8_t *payload,
                        uint32_t payloadSize) {
  display.showStage("VERIFY", "IMAGE");
  if (payloadSize < 4) {
    return LoadError::PayloadTooSmall;
  }

  const uint32_t firstWord =
      static_cast<uint32_t>(payload[0]) |
      (static_cast<uint32_t>(payload[1]) << 8) |
      (static_cast<uint32_t>(payload[2]) << 16) |
      (static_cast<uint32_t>(payload[3]) << 24);
  if (firstWord == 0) {
    return LoadError::ImageEmpty;
  }

  return LoadError::None;
}

void clearScratch(uint8_t *payload, uint32_t payloadSize) {
  for (uint32_t i = 0; i < payloadSize; ++i) {
    payload[i] = 0;
  }
}

void handleRecoverableError(Chainloader::Uart &uart,
                            Chainloader::BootDisplay &display,
                            LoadError error) {
  switch (error) {
  case LoadError::ShortSize:
    showRetry(display, "SHORT", "SIZE", "Short read while receiving size");
    break;
  case LoadError::InvalidSize:
    showRetry(display, "SIZE", "INVALID", "Invalid payload size");
    break;
  case LoadError::ShortPayload:
    showRetry(display, "SHORT", "PAYLOAD",
              "Timed out while receiving payload");
    break;
  case LoadError::PayloadTooSmall:
    showRetry(display, "SIZE", "SMALL", "Payload too small to execute");
    break;
  case LoadError::ImageEmpty:
    showRetry(display, "IMAGE", "EMPTY", "Payload entry bytes were zero");
    break;
  case LoadError::None:
    return;
  }

  uart.clearRx();
}

[[noreturn]] void relocateAndJump(Chainloader::BootDisplay &display,
                                  uint8_t *payload, uint32_t payloadSize) {
  char jumpLine[17];
  snprintf_(jumpLine, sizeof(jumpLine), "0x%08X",
            static_cast<unsigned int>(
                Chainloader::Board::PAYLOAD_ENTRY_ADDRESS));
  display.showStage("JUMP", jumpLine);

  uartLog("[chainloader] Relocating %u bytes to 0x%08X\n",
          static_cast<unsigned int>(payloadSize),
          static_cast<unsigned int>(Chainloader::Board::PAYLOAD_ENTRY_ADDRESS));

  alignas(16) uint8_t trampoline[128] = {};
  const size_t trampolineSize =
      static_cast<size_t>(&chainloader_relocator_end -
                          &chainloader_relocator_start);
  if (trampolineSize == 0 || trampolineSize > sizeof(trampoline)) {
    fatal(display, "JUMP", "TRAMP", "Invalid relocator trampoline size");
  }

  copyBytes(trampoline, &chainloader_relocator_start, trampolineSize);
  auto relocator = reinterpret_cast<RelocatorFn>(trampoline);

  relocator(reinterpret_cast<void *>(Chainloader::Board::PAYLOAD_ENTRY_ADDRESS),
            payload, payloadSize, Chainloader::Board::PAYLOAD_ENTRY_ADDRESS);

  fatal(display, "JUMP", "RETURN", "Payload unexpectedly returned");
}

} // namespace

extern "C" void putchar_(char c) {
  if (g_uart != nullptr) {
    g_uart->writeByte(c);
  }
}

extern "C" void chainloader_init() {
  Chainloader::GPIO gpio(Chainloader::Board::GPIO_BASE);
  Chainloader::LCD lcd(gpio);
  Chainloader::BootDisplay display(lcd);

  lcd.init();
  runLcdSelfTest(lcd);
  display.showStage("BOOT", "GPIO+LCD");

  gpio.mapUartPins();

  Chainloader::Uart uart(Chainloader::Board::UART_BASE);
  g_uart = &uart;

  display.showStage("UART", "INIT");
  uart.init();

  uartLog("\n[chainloader] LCD-aware UART chainloader\n");
  uartLog("[chainloader] Scratch=0x%08X Max=%u Entry=0x%08X\n",
          static_cast<unsigned int>(
              Chainloader::Board::PAYLOAD_SCRATCH_ADDRESS),
          static_cast<unsigned int>(Chainloader::Board::MAX_PAYLOAD_SIZE),
          static_cast<unsigned int>(Chainloader::Board::PAYLOAD_ENTRY_ADDRESS));

  uint8_t *payload =
      reinterpret_cast<uint8_t *>(Chainloader::Board::PAYLOAD_SCRATCH_ADDRESS);
  while (true) {
    const LoadAttempt attempt = waitForPayloadSize(uart, display);
    if (attempt.error != LoadError::None) {
      handleRecoverableError(uart, display, attempt.error);
      continue;
    }

    clearScratch(payload, attempt.payloadSize);

    LoadError error = receivePayload(uart, display, payload, attempt.payloadSize);
    if (error != LoadError::None) {
      handleRecoverableError(uart, display, error);
      continue;
    }

    error = verifyPayload(display, payload, attempt.payloadSize);
    if (error != LoadError::None) {
      handleRecoverableError(uart, display, error);
      continue;
    }

    relocateAndJump(display, payload, attempt.payloadSize);
  }
}
