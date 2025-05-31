#pragma once

namespace CPU {
void nop();
void spinForCycles(unsigned int cycles);
void enableInterrupts();
void disableInterrupts();
[[noreturn]] void waitForever();
} // namespace CPU
