#pragma once

#include <arch/aarch64/memory/registers/esr_el1.hpp>
#include <arch/aarch64/memory/registers/spsr_el1.hpp>
#include <exception.hpp>
#include <memory/memory_register.hpp>
#include <panic.hpp>
#include <stdio/printf.h>

namespace Exception {
namespace CurrentEL {
enum Value { EL2 = 0b10, EL1 = 0b01, EL0 = 0b00 };

class CurrentExLevel : public Memory::InMemoryRegister<CurrentExLevel> {
public:
  static Value get() {
    uint64_t value = 0;
    asm volatile("mrs %0, CurrentEL" : "=r"(value));
    return static_cast<Value>((value >> 2) &
                              0b11); // Bits [3:2] hold the EL value
  }

  static void set(uint64_t value) {
    panic("Cannot write to CurrentEL register");
  };
};
} // namespace CurrentEL

// ESR_EL1 Register

struct ExceptionContext {
  // General Purpose Registers
  uint64_t gpr[30];
  // The link register
  uint64_t lr;
  // Exception Link Register
  uint64_t elr_el1;
  // Saved Program Status Register
  Memory::Registers::SpsrEL1 spsr_el1;
  // Exception Syndrome Register
  Memory::Registers::EsrEL1 esr_el1 = Memory::Registers::EsrEL1();

  void print() {
    printf_("\nCPU Exception!\n\n");
    esr_el1.print();
    if (fault_address_valid()) {
      uint64_t far = 0;
      asm volatile("mrs %0, far_el1" : "=r"(far));
      printf_("FAR_EL1: 0x%018lX\n", far);
    }
    spsr_el1.print();
    printf_("\n");
    printf_("General purpose register:\n");
    for (uint32_t i = 0; i < 30; i++) {
      printf_("      0x%02d: 0x%018lX%s", i, gpr[i], i % 2 ? "\n" : "   ");
    }
    printf_("      lr: 0x%018lX\n", lr);
    printf_("\n");
  }

  bool fault_address_valid() const {
    uint64_t esr = (esr_el1.get() >> 26) & 0x3F;

    switch (esr) {
    case 0b100000: // InstrAbortLowerEL:
    case 0b100001: // InstrAbortCurrentEL:
    case 0b100010: // PCAlignmentFault:
    case 0b100100: // DataAbortLowerEL:
    case 0b100101: // DataAbortCurrentEL:
    case 0b110100: // WatchpointLowerEL:
    case 0b110101: // WatchpointCurrentEL:
      return true;

    default:
      return false;
    }
  }
};

extern "C" unsigned char __exception_vector_start;

} // namespace Exception
