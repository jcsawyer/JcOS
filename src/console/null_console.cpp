#include "console.hpp"
#include "null_console.hpp"
#include "printf.h"

namespace Console {
    void Console::flush() {}
    void Console::clearRx() {}
    void Console::print(const char* s, ...) {}
    void Console::printChar(char character) {}
    void Console::printLine(const char* format, ...) {}
    char Console::readChar() {
        return ' ';
    }
}