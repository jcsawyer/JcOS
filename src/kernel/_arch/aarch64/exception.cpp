#include "exception.hpp"
#include "../../exception.hpp"
#include <stdint.h>
#include <panic.hpp>

namespace Exception
{
  // default exception handler
  extern "C" void defaultExceptionHandler(ExceptionContext *context)
  {
    // TODO format CPU exception message
    panic("CPU Exception");
  }

  // Current, EL0
  extern "C" void current_el0_synchronous(ExceptionContext *context)
  {
    panic("Should not be here. Use of SP_EL0 in EL1 is not supoprted.");
  }

  extern "C" void current_el0_irq(ExceptionContext *context)
  {
    panic("Should not be here. Use of SP_EL0 in EL1 is not supported.");
  }

  extern "C" void current_el0_serror(ExceptionContext *context)
  {
    panic("Should not be here. Use of SP_EL0 in EL1 is not supported.");
  }

  // Current, ELx
  extern "C" void current_elx_synchronous(ExceptionContext *context)
  {
    defaultExceptionHandler(context);
  }

  extern "C" void current_elx_irq(ExceptionContext *context)
  {
    defaultExceptionHandler(context);
  }

  extern "C" void current_elx_serror(ExceptionContext *context)
  {
    defaultExceptionHandler(context);
  }

  // Lower, AArch64
  extern "C" void lower_aarch64_synchronous(ExceptionContext *context)
  {
    defaultExceptionHandler(context);
  }

  extern "C" void lower_aarch64_irq(ExceptionContext *context)
  {
    defaultExceptionHandler(context);
  }

  extern "C" void lower_aarch64_serror(ExceptionContext *context)
  {
    defaultExceptionHandler(context);
  }

  // Lower, AArch32
  extern "C" void lower_aarch32_synchronous(ExceptionContext *context)
  {
    defaultExceptionHandler(context);
  }

  extern "C" void lower_aarch32_irq(ExceptionContext *context)
  {
    defaultExceptionHandler(context);
  }

  extern "C" void lower_aarch32_serror(ExceptionContext *context)
  {
    defaultExceptionHandler(context);
  }

  void handlingInit()
  {

    //uint64_t vectorStart = reinterpret_cast<uint64_t>(&__exception_vector_start);
    //asm volatile("msr vbar_el1, %0" : : "r"(&vectorStart));

    // Force VBAR to update to complete before next instruction
    asm volatile("isb sy" ::: "memory");
  } // namespace Exception
}