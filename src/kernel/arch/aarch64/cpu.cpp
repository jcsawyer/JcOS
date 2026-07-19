#include "../cpu.hpp"

namespace CPU {
size_t currentCoreId() {
  unsigned long mpidr = 0;
  asm volatile("mrs %0, MPIDR_EL1" : "=r"(mpidr));
  return static_cast<size_t>(mpidr & 0b11);
}

void spinForCycles(unsigned int cycles) {
  for (unsigned int i = 0; i < cycles; i++) {
    asm volatile("nop");
  }
}

void nop() { asm volatile("nop"); }

void waitForEvent() { asm volatile("wfe" ::: "memory"); }

void sendEvent() { asm volatile("sev" ::: "memory"); }

void waitForever() {
  while (true) {
    waitForEvent();
  }
}

} // namespace CPU
