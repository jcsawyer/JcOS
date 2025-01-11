#include <main.hpp>
#include <stdint.h>

void prepare_el2_to_el1_transition(
    uint64_t phys_boot_core_stack_end_exclusive_addr) {
  // Enable timer counter registers for EL1
  asm volatile("msr CNTHCTL_EL2, %[value]"
               :
               : [value] "r"(0b11)); // EL1PCEN::SET + EL1PCTEN::SET

  // No offset for reading the counters
  asm volatile("msr CNTVOFF_EL2, %[value]" : : [value] "r"(0));

  // Set EL1 execution state to AArch64
  asm volatile("msr HCR_EL2, %[value]"
               :
               : [value] "r"(0b1 << 31)); // RW::EL1IsAarch64

  // Simulate exception return
  // Fake a saved program status where all interrupts are masked and SP_EL1 is
  // used as the stack pointer
  uint64_t spsr_value = 0b1 << 9 | // Mask D (Debug)
                        0b1 << 8 | // Mask A (SError)
                        0b1 << 7 | // Mask I (IRQ)
                        0b1 << 6 | // Mask F (FIQ)
                        0b0101;      // M::EL1h
  asm volatile("msr SPSR_EL2, %[value]" : : [value] "r"(spsr_value));

  // Set the link register to point to kernel_init
  asm volatile("msr ELR_EL2, %[value]"
               :
               : [value] "r"(reinterpret_cast<uint64_t>(&kernel_init)));

  // Set up SP_EL1 (stack pointer)
  asm volatile("msr SP_EL1, %[value]"
               :
               : [value] "r"(phys_boot_core_stack_end_exclusive_addr));

  uint64_t elr, spsr, sp_el1;

  asm volatile("mrs %0, ELR_EL2" : "=r"(elr));
  asm volatile("mrs %0, SPSR_EL2" : "=r"(spsr));
  asm volatile("mrs %0, SP_EL1" : "=r"(sp_el1));
}

extern "C" void _start_cpp(uint64_t phys_boot_core_stack_end_exclusive_addr) {
  prepare_el2_to_el1_transition(phys_boot_core_stack_end_exclusive_addr);

  asm volatile("eret");
}