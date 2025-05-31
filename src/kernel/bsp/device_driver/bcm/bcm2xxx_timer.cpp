#include "bcm2xxx_timer.hpp"
#include <arch/cpu.hpp>
#include <print.hpp>
#include <stdint.h>
#include <task.hpp>

#define IRQ_ENABLE1 ((volatile uint32_t *)(0x3F00B210))
#define TIMER_CS ((volatile uint32_t *)(0x3F003000))
#define TIMER_CLO ((volatile uint32_t *)(0x3F003004))
#define TIMER_C1 ((volatile uint32_t *)(0x3F003010))

#define TIMER_IRQ_1 (1 << 1)

namespace Driver::BSP::BCM {

void Timer::init() {}

void Timer::timer_init() {
  *TIMER_CS = TIMER_IRQ_1;        // Clear any pending interrupts
  *TIMER_C1 = *TIMER_CLO + 10000; // Set next timer event in ~10ms
  *IRQ_ENABLE1 = TIMER_IRQ_1;     // Enable timer IRQ
  CPU::enableInterrupts();        // Enable interrupts globally
}

extern "C" void current_elx_irq(uint64_t *context) {
  if (*TIMER_CS & TIMER_IRQ_1) {
    *TIMER_CS = TIMER_IRQ_1;        // Clear the interrupt
    *TIMER_C1 = *TIMER_CLO + 10000; // Schedule next timer IRQ
  }

  asm volatile("msr daifclr, #2" ::: "memory");

  // Call the schedule, which handles everything including switching
  taskManager.schedule();

  // No more code here — schedule() does the context switch and never returns
}
} // namespace Driver::BSP::BCM