#include "qemu_console.hpp"
#include <stdarg.h>

extern "C" void vprintf_(const char *format, ...);

namespace Console {
void QemuConsole::flush() {}

void QemuConsole::clearRx() {}

void QemuConsole::print(const char *s, ...) {
  for (int i = 0; s[i] != '\0'; i++) {
    printChar(s[i]);
  }
}

void QemuConsole::printChar(const char character) {
  const auto address = reinterpret_cast<volatile char *>(0x3F201000);
  *address = static_cast<char>(character);
}

void QemuConsole::printLine(const char *format, ...) {
  va_list args;
  va_start(args, format);

  // zwvprintf_(format, args);

  va_end(args);

  printChar('\n'); // Print a newline at the end
}

char QemuConsole::readChar() {
  const auto uartAddress = reinterpret_cast<volatile char *>(0x3F201000);
  return *uartAddress; // Read character from UART
}
} // namespace Console
