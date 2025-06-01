#include "bcm2xxx_pl011_uart.hpp"
#include "../../../arch/cpu.hpp"
#include <exceptions/asynchronous.hpp>
#include <stdio/printf.h>

namespace Driver::BSP::BCM {
/// Set up baud rate and characteristics.
///
/// This results in 8N1 and 921_600 baud.
///
/// The calculation for the BRD is (we set the clock to 48 MHz in config.txt):
/// `(48_000_000 / 16) / 921_600 = 3.2552083`.
///
/// This means the integer part is `3` and goes into the `IBRD`.
/// The fractional part is `0.2552083`.
///
/// `FBRD` calculation according to the PL011 Technical Reference Manual:
/// `INTEGER((0.2552083 * 64) + 0.5) = 16`.
///
/// Therefore, the generated baud rate divider is: `3 + 16/64 = 3.25`. Which
/// results in a genrated baud rate of `48_000_000 / (16 * 3.25) = 923_077`.
///
/// Error = `((923_077 - 921_600) / 921_600) * 100 = 0.16%`.
void UART::init() {
  flush();

  // // Disable UART0
  *registerBlock.CR = 0;

  *registerBlock.ICR = 0b00000000000;

  *registerBlock.IBRD = 3;
  *registerBlock.FBRD = 16;

  *registerBlock.LCRH = 1 << 4 | 0b11 << 5;

  *registerBlock.IFLS = 0b000; // Set TX and RX FIFO levels (one eighth)
  *registerBlock.IMSC = 1 << 4 | 1 << 6; // Enable RX and TX interrupts

  *registerBlock.CR = 1 << 0 | 1 << 8 | 1 << 9;
}

void UART::registerAndEnableIrqHandler(
    ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) {

  Exceptions::Asynchronous::IRQHandlerDescriptor descriptor(
      irqNumber->kind, *irqNumber, compatible(),
      const_cast<IRQHandler *>(static_cast<const IRQHandler *>(this)));

  // Register and enable via irq_manager singleton
  Exceptions::Asynchronous::IRQManager *irqManager =
      Exceptions::Asynchronous::irq_manager();
  irqManager->registerHandler(descriptor);
  irqManager->enable(irqNumber);
}

void UART::putc(const char c) const {
  while (*registerBlock.FR & 0x20) {
    // Wait for the UART to become ready to transmit.
  }
  *registerBlock.DR = c;
}

Optional<char> UART::getc(Console::Console::BlockingMode blockingMode) const {
  if (*registerBlock.FR & 0x10) {
    if (blockingMode == Console::Console::BlockingMode::NonBlocking) {
      return Optional<char>('\0'); // No character available
    }

    // If blocking mode is set, wait until a character is available
    while (*registerBlock.FR & 0x10) {
      CPU::nop(); // Wait for the UART to become ready to receive.
    }
  }

  const char c = *registerBlock.DR;

  // TODO add characters read/written counts
  return c;
}

void UART::flush() const {
  while (*registerBlock.FR & 0x20) {
    CPU::nop();
  }
}

void UART::UartConsole::flush() {}

void UART::UartConsole::clearRx() {}

void UART::UartConsole::print(const char *format, ...) {
  va_list args;
  va_start(args, format);

  vprintf_(format, args);

  va_end(args);
}

void UART::UartConsole::printChar(const char character) {
  uart->putc(character);
}

void UART::UartConsole::printLine(const char *format, ...) {
  va_list args;
  va_start(args, format);

  vprintf_(format, args);

  uart->putc('\n');
  va_end(args);
}

Optional<char> UART::UartConsole::readChar(Console::BlockingMode blockingMode) {
  return uart->getc(blockingMode);
}

bool UART::handle() {

  lock().lock([](UART &inner) {
    auto pending = *inner.registerBlock.MIS;

    // Clear all pending IRQs
    *inner.registerBlock.ICR = (1 << 1);

    // Check for any kind of RX interrupt
    if (pending & (1 << 4 | 1 << 6)) {
      // Echo any received characters.
      while (true) {
        auto opt = inner.getc(Console::Console::BlockingMode::NonBlocking);
        if (!opt.has_value())
          return false;
        inner.putc(opt.value());
      }
    }

    return true;
  });
}
} // namespace Driver::BSP::BCM
