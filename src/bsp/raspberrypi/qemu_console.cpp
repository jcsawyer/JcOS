#include "console.hpp"
#include "qemu_console.hpp"
#include "printf.h"

namespace Console {
    void Console::flush() {}
    void Console::clearRx() {}
    void Console::print(const char* s, ...) {
        for (int i = 0; s[i] != '\0'; i++) {
            printChar(s[i]);
        }
    }

    void Console::printChar(char character) {
        volatile char* address = reinterpret_cast<volatile char*>(0x3F201000);
            *address = static_cast<char>(character);
    }

    void Console::printLine(const char* format, ...) {
        va_list args;
        va_start(args, format);

        vprintf_(format, args);

        va_end(args);

        printChar('\n'); // Print a newline at the end
    }

    char Console::readChar() {
        volatile char* uartAddress = reinterpret_cast<volatile char*>(0x3F201000);
        return *uartAddress; // Read character from UART
    }
}