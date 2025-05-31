#include <print.hpp>
#include <stdint.h>
#include <task.hpp>

#define CNTP_CTL_EL0_ENABLE (1 << 0)
#define CNTP_CTL_EL0_IMASK (1 << 1)
#define CNTP_CTL_EL0_ISTATUS (1 << 2)

static inline unsigned long read_cntfrq_el0() {
  unsigned long val;
  asm volatile("mrs %0, cntfrq_el0" : "=r"(val));
  return val;
}

static inline void write_cntp_tval_el0(unsigned long val) {
  asm volatile("msr cntp_tval_el0, %0" ::"r"(val));
}

static inline void write_cntp_ctl_el0(unsigned long val) {
  asm volatile("msr cntp_ctl_el0, %0" ::"r"(val));
}

static inline void enable_timer_irq() {
  write_cntp_tval_el0(read_cntfrq_el0() / 10); // 100ms
  write_cntp_ctl_el0(CNTP_CTL_EL0_ENABLE);
}

// void timer_irq_handler() {
//   write_cntp_tval_el0(read_cntfrq_el0() / 10); // reset timer
//   task_schedule();                             // switch task
// }

void timerInit() {
  unsigned long freq;
  asm volatile("mrs %0, cntfrq_el0" : "=r"(freq));
  asm volatile("msr cntp_tval_el0, %0" ::"r"(freq / 10)); // 100ms
  asm volatile("msr cntp_ctl_el0, %0" ::"r"(1));          // enable timer
  asm volatile(
      "msr cntv_ctl_el0, %0" ::"r"(1)); // enable virtual timer (optional)
  asm volatile("isb");
  enable_timer_irq();
}