#include "main.hpp"

#include "board_config.hpp"
#include "boot_display.hpp"
#include "gpio.hpp"
#include "lcd.hpp"
#include "uart.hpp"

#include <arch/cpu.hpp>
#include <stdint.h>
#include <stdio/printf.h>

namespace {

extern "C" uint8_t chainloader_relocator_start;
extern "C" uint8_t chainloader_relocator_end;

using RelocatorFn = void (*)(void *destination, const void *source, size_t size,
                             uintptr_t entryAddress,
                             uintptr_t deviceTreePhysAddr);

void copyBytes(void *destination, const void *source, size_t size) {
  auto *dst = static_cast<uint8_t *>(destination);
  auto *src = static_cast<const uint8_t *>(source);
  for (size_t i = 0; i < size; ++i) {
    dst[i] = src[i];
  }
}

[[noreturn]] void fatal(Chainloader::BootDisplay &display, const char *code,
                        const char *detail) {
  display.showError(code, detail);
  CPU::waitForever();
}

char readByte(Chainloader::Uart &uart) {
  char value = 0;
  while (!uart.readByte(value)) {
    CPU::nop();
  }
  return value;
}

uint32_t readPayloadSize(Chainloader::Uart &uart) {
  uint32_t size = 0;
  for (unsigned int i = 0; i < 4; ++i) {
    size |= static_cast<uint32_t>(static_cast<uint8_t>(readByte(uart)))
            << (i * 8);
  }
  return size;
}

[[noreturn]] void relocateAndJump(Chainloader::BootDisplay &display,
                                  uint8_t *payload, uint32_t payloadSize,
                                  uintptr_t deviceTreePhysAddr) {
  char jumpLine[17];
  snprintf_(
      jumpLine, sizeof(jumpLine), "0x%08X",
      static_cast<unsigned int>(Chainloader::Board::PAYLOAD_ENTRY_ADDRESS));
  display.showStage("JUMP", jumpLine);

  alignas(16) uint8_t trampoline[128] = {};
  const size_t trampolineSize = static_cast<size_t>(
      &chainloader_relocator_end - &chainloader_relocator_start);
  if (trampolineSize == 0 || trampolineSize > sizeof(trampoline)) {
    fatal(display, "JUMP", "TRAMP");
  }

  copyBytes(trampoline, &chainloader_relocator_start, trampolineSize);
  auto relocator = reinterpret_cast<RelocatorFn>(trampoline);

  relocator(reinterpret_cast<void *>(Chainloader::Board::PAYLOAD_ENTRY_ADDRESS),
            payload, payloadSize, Chainloader::Board::PAYLOAD_ENTRY_ADDRESS,
            deviceTreePhysAddr);

  fatal(display, "JUMP", "RETURN");
}

} // namespace

extern "C" void putchar_(char) {}

extern "C" void chainloader_init(uintptr_t deviceTreePhysAddr) {
  Chainloader::GPIO gpio(Chainloader::Board::GPIO_BASE);
  Chainloader::LCD lcd(gpio);
  Chainloader::BootDisplay display(lcd);

  lcd.init();
  display.showStage("RASPBOOTIN", "LCD READY");

  gpio.mapUartPins();

  Chainloader::Uart uart(Chainloader::Board::UART_BASE);

  display.showStage("UART", "INIT");
  uart.init();

  uint8_t *payload =
      reinterpret_cast<uint8_t *>(Chainloader::Board::PAYLOAD_SCRATCH_ADDRESS);
  while (true) {
    display.showStage("WAIT", "REQUEST");
    uart.writeByte(0x03);
    uart.writeByte(0x03);
    uart.writeByte(0x03);

    const uint32_t payloadSize = readPayloadSize(uart);
    if (payloadSize == 0 ||
        payloadSize > Chainloader::Board::MAX_PAYLOAD_SIZE) {
      display.showError("SIZE", "REJECT");
      uart.writeByte('S');
      uart.writeByte('E');
      continue;
    }

    uart.writeByte('O');
    uart.writeByte('K');

    display.showStage("RX", "PAYLOAD");
    for (uint32_t i = 0; i < payloadSize; ++i) {
      payload[i] = static_cast<uint8_t>(readByte(uart));
    }

    display.showStage("BOOT", "PAYLOAD");
    uart.waitForTxIdle();
    relocateAndJump(display, payload, payloadSize, deviceTreePhysAddr);
  }
}
