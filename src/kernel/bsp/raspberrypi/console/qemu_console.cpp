#include "qemu_console.hpp"
#include <optional.hpp>
#include <stdarg.h>
#include <stdio/printf.h>

namespace Console {
void QemuConsole::flush() {}

void QemuConsole::clearRx() {}

void QemuConsole::print(const char *format, ...) {
  va_list args;
  va_start(args, format);

  vprintf_(format, args);

  va_end(args);
}

void QemuConsole::printChar(const char character) {
  const auto address = reinterpret_cast<volatile char *>(0x3F201000);
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
  const auto uartAddress = reinterpret_cast<volatile char *>(0x3F201000);
  return Optional<char>(
      static_cast<char>(*uartAddress)); // Read character from UART
}

} // namespace Console
