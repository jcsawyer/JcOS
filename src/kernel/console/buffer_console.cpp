#include "buffer_console.hpp"

#include <stdarg.h>
#include <stdio/printf.h>

namespace Console {
namespace {
extern "C" void bufferConsolePutChar(char c, void *extraArg) {
  BufferConsole *console = reinterpret_cast<BufferConsole *>(extraArg);
  console->printChar(c);
}
} // namespace

void BufferConsole::flush() {}

void BufferConsole::clearRx() {}

void BufferConsole::appendChar(char c) {
  if (size < BUFFER_SIZE) {
    buffer[size++] = c;
  }
}

void BufferConsole::print(const char *format, ...) {
  va_list args;
  va_start(args, format);

  vfctprintf(bufferConsolePutChar, this, format, args);

  va_end(args);
}

void BufferConsole::printChar(char c) { appendChar(c); }

void BufferConsole::printLine(const char *format, ...) {
  va_list args;
  va_start(args, format);

  vfctprintf(bufferConsolePutChar, this, format, args);
  appendChar('\n');

  va_end(args);
}

Optional<char> BufferConsole::readChar(Console::BlockingMode blockingMode) {
  (void)blockingMode;
  return Optional<char>();
}

void BufferConsole::dumpTo(Console &target) {
  for (size_t i = 0; i < size; ++i) {
    target.printChar(buffer[i]);
  }
  size = 0;
}

} // namespace Console
