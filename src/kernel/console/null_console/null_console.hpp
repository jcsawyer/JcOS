#pragma once
#include "../console.hpp"
#include <optional.hpp>

namespace Console {
class NullConsole : public Console {
public:
  void flush() override;
  void clearRx() override;
  void print(const char *s, ...) override;
  void printChar(char c) override;
  void printLine(const char *s, ...) override;
  Optional<char> readChar(Console::BlockingMode blockingMode) override;
};
} // namespace Console
