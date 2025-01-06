#include "console.hpp"
#include "../std/stddef.h"
#include "null_console/null_console.hpp"

namespace Console {

Console *Console::instance = nullptr;

Console *Console::GetInstance() {
  if (instance == nullptr) {
    static NullConsole nullConsole;
    instance = &nullConsole;
  }
  return instance;
}

void Console::SetInstance(Console *newConsole) { instance = newConsole; }
} // namespace Console
