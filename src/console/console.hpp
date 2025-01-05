#pragma once

namespace Console {
    class Console {
    public:
        virtual void flush() = 0;
        virtual void clearRx() = 0;
        virtual void print(const char* s, ...) = 0;
        virtual void printChar(char c) = 0;
        virtual void printLine(const char* s, ...) = 0;
        virtual char readChar() = 0;
    };

    Console& console();
    void setConsole(Console* newConsole);
}
