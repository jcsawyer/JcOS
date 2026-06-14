#include "console.hpp"
#include "buffer_console.hpp"

namespace Console {

Console *Console::instance = nullptr;

Console *Console::GetInstance() {
  if (instance == nullptr) {
    static BufferConsole bufferConsole;
    instance = &bufferConsole;
  }
  return instance;
}

void Console::SetInstance(Console *newConsole) {
  Console *current = GetInstance();
  if (current != newConsole) {
    current->dumpTo(*newConsole);
  }
  instance = newConsole;
}
} // namespace Console
