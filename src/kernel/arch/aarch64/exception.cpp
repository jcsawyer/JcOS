#include "exception.hpp"
#include "../../exception.hpp"
#include <panic.hpp>
#include <stdint.h>

namespace Exception {
// default exception handler
extern "C" void defaultExceptionHandler(ExceptionContext *context) {
  // TODO format CPU exception message
  context->print();
  panic("HALTING");
}

// Current, EL0
extern "C" void current_el0_synchronous(ExceptionContext *context) {
  panic("Should not be here. Use of SP_EL0 in EL1 is not supoprted.");
}

extern "C" void current_el0_irq(ExceptionContext *context) {
  panic("Should not be here. Use of SP_EL0 in EL1 is not supported.");
}

extern "C" void current_el0_serror(ExceptionContext *context) {
  panic("Should not be here. Use of SP_EL0 in EL1 is not supported.");
}

// Current, ELx
extern "C" void current_elx_synchronous(ExceptionContext *context) {
  if (context->fault_address_valid()) {
    // get the value of FAR_EL1
    uint64_t far = 0;
    asm volatile("mrs %0, far_el1" : "=r"(far));
  }

  defaultExceptionHandler(context);
}

extern "C" void current_elx_irq(ExceptionContext *context) {
  defaultExceptionHandler(context);
}

extern "C" void current_elx_serror(ExceptionContext *context) {
  defaultExceptionHandler(context);
}

// Lower, AArch64
extern "C" void lower_aarch64_synchronous(ExceptionContext *context) {
  defaultExceptionHandler(context);
}

extern "C" void lower_aarch64_irq(ExceptionContext *context) {
  defaultExceptionHandler(context);
}

extern "C" void lower_aarch64_serror(ExceptionContext *context) {
  defaultExceptionHandler(context);
}

// Lower, AArch32
extern "C" void lower_aarch32_synchronous(ExceptionContext *context) {
  defaultExceptionHandler(context);
}

extern "C" void lower_aarch32_irq(ExceptionContext *context) {
  defaultExceptionHandler(context);
}

extern "C" void lower_aarch32_serror(ExceptionContext *context) {
  defaultExceptionHandler(context);
}

PrivilegeLevel current_privilege_level(const char **el_string) {
  CurrentEL::Value el = CurrentEL::CurrentExLevel::get();
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

void handlingInit() {

  uint64_t vector_base = reinterpret_cast<uint64_t>(&__exception_vector_start);

  // Write to VBAR_EL1 so the CPU uses that address for exception vectors.
  asm volatile("msr VBAR_EL1, %0" : : "r"(vector_base));
  asm volatile("isb"); // Force completion before next instruction
} // namespace Exception
} // namespace Exception