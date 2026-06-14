#pragma once

#include "console.hpp"

namespace Console {

class BufferConsole : public Console {
public:
  void flush() override;
  void clearRx() override;
  void print(const char *format, ...) override;
  void printChar(char c) override;
  void printLine(const char *format, ...) override;
  Optional<char> readChar(Console::BlockingMode blockingMode) override;
  void dumpTo(Console &target) override;

private:
  static constexpr size_t BUFFER_SIZE = 8192;

  void appendChar(char c);

  char buffer[BUFFER_SIZE] = {};
  size_t size = 0;
};

} // namespace Console
