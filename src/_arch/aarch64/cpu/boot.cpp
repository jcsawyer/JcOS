#include "main.hpp"
#include "console.hpp"
#include "null_console.hpp"

extern "C" void _start_cpp() {
    Console::setConsole(Console::NullConsole());
    
    kernel_init();
}