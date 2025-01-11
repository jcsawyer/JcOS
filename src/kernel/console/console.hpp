#pragma once

namespace Console {
class Console {
public:
  static Console *GetInstance();
  static void SetInstance(Console *newConsole);

  virtual void flush() = 0;
  virtual void clearRx() = 0;
  virtual void print(const char *format, ...) = 0;
  virtual void printChar(char character) = 0;
  virtual void printLine(const char *format, ...) = 0;
  virtual char readChar() = 0;

protected:
  Console() = default;
  ~Console() = default;

private:
  static Console *instance;
};
} // namespace Console
