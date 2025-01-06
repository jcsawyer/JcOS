#pragma once
#include "console/console.hpp"

template <class... Args> void info(const char *format, Args... args) {
  Console::Console *console = Console::Console::GetInstance();

  console->print("[ %03d.%06d ] ", 0, 0);
  console->print(format, args...);
  console->print("\n");
}
