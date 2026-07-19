#pragma once

#include <stddef.h>

namespace CPU {
size_t currentCoreId();
void nop();
void spinForCycles(unsigned int cycles);
void enableInterrupts();
void disableInterrupts();
void waitForEvent();
void sendEvent();
[[noreturn]] void waitForever();
} // namespace CPU
