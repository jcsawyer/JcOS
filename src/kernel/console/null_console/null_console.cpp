#include "null_console.hpp"

namespace Console {
void NullConsole::flush() {}
void NullConsole::clearRx() {}
void NullConsole::print(const char *s, ...) {}
void NullConsole::printChar(char character) {}
void NullConsole::printLine(const char *format, ...) {}
Optional<char> NullConsole::readChar(Console::BlockingMode blockingMode) {
  return Optional<char>('\0');
}
} // namespace Console
