#include "console.hpp"

namespace Console {

    static Console* _console;

    Console& console() {
        return *_console;
    }

    void setConsole(Console* newConsole) {
        if (newConsole != _console) {
            _console = newConsole;
        }
    }

}
