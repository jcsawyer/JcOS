#include "exception.hpp"
#include "../../exception.hpp"
#include "../../std/stdint.h"

namespace Exception {
namespace CurrentEL {
enum Value { EL2 = 0b10, EL1 = 0b01, EL0 = 0b00 };

// Reads the CurrentEL register and extracts the EL field
Value read_el() {
  uint64_t value;
  asm volatile("mrs %0, CurrentEL" : "=r"(value));
  return static_cast<Value>((value >> 2) &
                            0b11); // Bits [3:2] hold the EL value
}
} // namespace CurrentEL

// Determines the current privilege level
PrivilegeLevel current_privilege_level(const char **el_string) {
  CurrentEL::Value el = CurrentEL::read_el();
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
} // namespace Exception
