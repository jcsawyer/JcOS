#pragma once
#include <memory/memory_register.hpp>
#include <stdio/printf.h>

namespace Memory::Registers {

class SpsrEL1 : public Memory::InMemoryRegister<SpsrEL1> {
public:
  static uint64_t get() {
    uint64_t value = 0;
    asm volatile("mrs %0, spsr_el1" : "=r"(value));
    return value;
  };

  static void set(uint64_t value) {
    asm volatile("msr spsr_el1, %0" : : "r"(value));
  };

  static void print() {
    printf_("SPSR_EL1: 0x%08lX\n", get());
    printf_("      Flags:\n");
    printf_("            Negative: (N) %s\n", toFlatStr(get() & 1 << 31));
    printf_("            Zero:     (Z) %s\n", toFlatStr(get() & 1 << 30));
    printf_("            Carry:    (C) %s\n", toFlatStr(get() & 1 << 29));
    printf_("            Overflow: (V) %s\n", toFlatStr(get() & 1 << 28));

    printf_("      Exception handling state:\n");
    printf_("            Debug  (D): %s\n", toMaskStr(get() & 1 << 9));
    printf_("            SError (A): %s\n", toMaskStr(get() & 1 << 8));
    printf_("            IRQ    (I): %s\n", toMaskStr(get() & 1 << 7));
    printf_("            FIQ    (F): %s\n", toMaskStr(get() & 1 << 6));

    printf_("      Illegal Execution State (IL): %s\n",
            toFlatStr(get() & 1 << 20));
  }

  static const char *toFlatStr(uint64_t value) {
    if (value)
      return "Set";
    return "Not set";
  }
  static const char *toMaskStr(uint64_t value) {
    if (value)
      return "Masked";
    return "Unmasked";
  }
};
} // namespace Memory::Registers
