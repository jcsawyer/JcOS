#include "console.hpp"
#include "null_console.hpp"
#include "printf.h"

namespace Console {
    void NullConsole::flush() {}
    void NullConsole::clearRx() {}
    void NullConsole::print(const char* s, ...) {}
    void NullConsole::printChar(char character) {}
    void NullConsole::printLine(const char* format, ...) {}
    char NullConsole::readChar() {
        return '\0';
    }
}