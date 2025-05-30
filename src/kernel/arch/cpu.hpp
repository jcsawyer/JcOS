#pragma once

namespace CPU {
void nop();
void spinForCycles(unsigned int cycles);
void enableInterrupts();
[[noreturn]] void waitForever();
} // namespace CPU
