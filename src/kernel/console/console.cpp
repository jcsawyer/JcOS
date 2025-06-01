#include "console.hpp"
#include "bsp/raspberrypi/console/qemu_console.hpp"
#include "null_console/null_console.hpp"

namespace Console {

Console *Console::instance = nullptr;

Console *Console::GetInstance() {
  if (instance == nullptr) {
    static QemuConsole nullConsole;
    instance = &nullConsole;
  }
  return instance;
}

void Console::SetInstance(Console *newConsole) { instance = newConsole; }
} // namespace Console
