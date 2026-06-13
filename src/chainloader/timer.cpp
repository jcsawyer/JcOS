#include "timer.hpp"

#include <arch/cpu.hpp>

extern "C" {
__attribute__((used, visibility("default"))) extern uint64_t
    ARCH_TIMER_COUNTER_FREQUENCY asm("ARCH_TIMER_COUNTER_FREQUENCY") = 0;
}

namespace Chainloader {

uint32_t Timer::frequency() {
  return static_cast<uint32_t>(ARCH_TIMER_COUNTER_FREQUENCY);
}

uint64_t Timer::ticks() {
  uint64_t counter = 0;
  asm volatile("mrs %0, cntpct_el0" : "=r"(counter));
  return counter;
}

uint64_t Timer::microsToTicks(uint64_t micros) {
  const uint64_t freq = frequency();
  return (micros * freq) / 1000000;
}

bool Timer::expired(uint64_t startTicks, uint64_t timeoutMicros) {
  return ticks() - startTicks >= microsToTicks(timeoutMicros);
}

void Timer::delayMicros(uint64_t micros) {
  const uint64_t start = ticks();
  const uint64_t waitTicks = microsToTicks(micros);
  while (ticks() - start < waitTicks) {
    CPU::nop();
  }
}

void Timer::delayMillis(uint64_t millis) { delayMicros(millis * 1000); }

} // namespace Chainloader
