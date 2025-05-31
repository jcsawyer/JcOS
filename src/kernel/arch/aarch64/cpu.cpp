#include "../cpu.hpp"

namespace CPU {
void spinForCycles(unsigned int cycles) {
  for (unsigned int i = 0; i < cycles; i++) {
    asm volatile("nop");
  }
}

void nop() { asm volatile("nop"); }

void waitForever() {
  while (true) {
    asm volatile("wfe");
  }
}

} // namespace CPU
