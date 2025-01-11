#pragma once
#include <memory/memory_register.hpp>
#include <stdio/printf.h>

namespace Memory::Registers {
class EsrEL1 : public Memory::InMemoryRegister<EsrEL1> {
public:
  static uint64_t get() {
    uint64_t value = 0;
    asm volatile("mrs %0, esr_el1" : "=r"(value));
    return value;
  };

  static void set(uint64_t value) {
    asm volatile("msr esr_el1, %0" : : "r"(value));
  };

  static void print() {
    printf_("ESR_EL1: 0x%lX\n", get());
    printf_("      Exception Class         (EC) : 0x%lX", get() >> 26);
    switch (get() >> 26) {
    case 0b000001:
      printf_(" - Trapped WFI/WFE");
      break;
    case 0b001110:
      printf_(" - Illegal execution");
      break;
    case 0b010101:
      printf_(" - System call");
      break;
    case 0b100000:
      printf_(" - Instruction abort, Lower EL");
      break;
    case 0b100001:
      printf_(" - Instruction abort, Current EL");
      break;
    case 0b100010:
      printf_(" - Instruction alignment fault");
      break;
    case 0b100100:
      printf_(" - Data abort, Lower EL");
      break;
    case 0b100101:
      printf_(" - Data abort, Current EL");
      break;
    case 0b100110:
      printf_(" - Stack alignment fault");
      break;
    case 0b101100:
      printf_(" - Floating point");
      break;
    default:
      printf_(" - Unknown");
      break;
    }
    printf_("\n");

    printf_("      Instr Specific Syndrome (ISS): 0x%lX\n", get() & 0x1ffffff);
  }
};
} // namespace Memory::Registers