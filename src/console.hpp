#pragma once

namespace Console {
    class Console {
    public:
        virtual void flush();
        virtual void clearRx();
        virtual void print(const char* s, ...);
        virtual void printChar(char c);
        virtual void printLine(const char* s, ...);
        virtual char readChar();
    protected:
        int bytes_written = 0;
        int bytes_read = 0;
    };

    inline static Console console;

    static void setConsole(Console newConsole) {
        console = newConsole;
    }
}
