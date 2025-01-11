#pragma once

#include "../../exception.hpp"
#include <memory/memory_register.hpp>
#include <panic.hpp>
#include <stdio/printf.h>

namespace Exception {
namespace CurrentEL {
enum Value { EL2 = 0b10, EL1 = 0b01, EL0 = 0b00 };
class CurrentExLevel : public Memory::InMemoryRegister<CurrentExLevel> {
public:
  static Exception::CurrentEL::Value get() {
    uint64_t value = 0;
    asm volatile("mrs %0, CurrentEL" : "=r"(value));
    return static_cast<Exception::CurrentEL::Value>(
        (value >> 2) & 0b11); // Bits [3:2] hold the EL value
  }

  static void set(uint64_t value) {
    panic("Cannot write to CurrentEL register");
  };
};

// Determines the current privilege level
inline PrivilegeLevel current_privilege_level(const char **el_string) {
  CurrentEL::Value el = CurrentExLevel::get();
  switch (el) {
  case CurrentEL::EL2:
    *el_string = "EL2";
    return PrivilegeLevel::Hypervisor;
  case CurrentEL::EL1:
    *el_string = "EL1";
    return PrivilegeLevel::Kernel;
  case CurrentEL::EL0:
    *el_string = "EL0";
    return PrivilegeLevel::User;
  default:
    *el_string = "Unknown";
    return PrivilegeLevel::Unknown;
  }
}
} // namespace CurrentEL

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

// ESR_EL1 Register
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
} __attribute__((packed));

struct ExceptionContext {
  // General Purpose Registers
  uint64_t gpr[30];
  // The link register
  uint64_t lr;
  // Exception Link Register
  uint64_t elr_el1;
  // Saved Program Status Register
  SpsrEL1 spsr_el1;
  // Exception Syndrome Register
  EsrEL1 esr_el1 = EsrEL1();

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

void handlingInit();

} // namespace Exception
