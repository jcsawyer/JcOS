#include "console.hpp"
#include "qemu_console.hpp"

namespace Console {
    static Console console = QemuConsole();
}