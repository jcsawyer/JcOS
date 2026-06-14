#include "qemu_console.hpp"
#include "../memory.hpp"
#include <optional.hpp>
#include <stdarg.h>
#include <stdio/printf.h>

namespace Console {
namespace {
volatile char *qemuUartAddress() {
  return reinterpret_cast<volatile char *>(Memory::mmioRemapStart() +
                                           Memory::Map::UART_OFFSET);
}
} // namespace

void QemuConsole::flush() {}

void QemuConsole::clearRx() {}

void QemuConsole::print(const char *format, ...) {
  va_list args;
  va_start(args, format);

  vprintf_(format, args);

  va_end(args);
}

void QemuConsole::printChar(const char character) {
  const auto address = qemuUartAddress();
  *address = static_cast<char>(character);
}

void QemuConsole::printLine(const char *format, ...) {
  va_list args;
  va_start(args, format);

  vprintf_(format, args);

  printChar('\n'); // Print a newline at the end
  va_end(args);
}

Optional<char> QemuConsole::readChar(Console::BlockingMode blockingMode) {
  const auto uartAddress = qemuUartAddress();
  return Optional<char>(
      static_cast<char>(*uartAddress)); // Read character from UART
}

} // namespace Console
