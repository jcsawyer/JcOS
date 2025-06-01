#pragma once

#include <optional.hpp>

namespace Console {
class Console {
public:
  static Console *GetInstance();
  static void SetInstance(Console *newConsole);

  enum BlockingMode {
    Blocking,   // Wait until a character is available
    NonBlocking // Return immediately if no character is available
  };

  bool isDebugMode = true;
  virtual void flush() = 0;
  virtual void clearRx() = 0;
  virtual void print(const char *format, ...) = 0;
  virtual void printChar(char character) = 0;
  virtual void printLine(const char *format, ...) = 0;
  virtual Optional<char> readChar(Console::BlockingMode blockingMode) = 0;

protected:
  Console() = default;
  ~Console() = default;

private:
  static Console *instance;
};
} // namespace Console
