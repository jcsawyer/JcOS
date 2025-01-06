#pragma once

namespace CPU {
void nop();
void spinForCycles(unsigned int cycles);
[[noreturn]] void waitForever();
} // namespace CPU
