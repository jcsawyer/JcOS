#pragma once

namespace Console {
class Console {
public:
  static Console *GetInstance();
  static void SetInstance(Console *newConsole);

  virtual void flush() = 0;
  virtual void clearRx() = 0;
  virtual void print(const char *s, ...) = 0;
  virtual void printChar(char c) = 0;
  virtual void printLine(const char *s, ...) = 0;
  virtual char readChar() = 0;

protected:
  Console() = default;
  ~Console() = default;

private:
  static Console *instance;
};
} // namespace Console
