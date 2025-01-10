#pragma once

#include "../../../console/console.hpp"

namespace Console {
class QemuConsole : public Console {
public:
  void flush() override;
  void clearRx() override;
  void print(const char *s, ...) override;
  void printChar(char c) override;
  void printLine(const char *s, ...) override;
  char readChar() override;
};
} // namespace Console
