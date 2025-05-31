#include "asynchronous.hpp"
#include "../../../print.hpp"
#include <exceptions/asynchronous.hpp>

namespace Exception {

bool is_set(unsigned long field) {
  unsigned long daif_value;
  asm volatile("mrs %0, DAIF" : "=r"(daif_value));
  return (daif_value & field) != 0;
}

const char *to_mask_str(const bool is_masked) {
  return is_masked ? "Masked" : "Unmasked";
}

void print_state() {
  info("      Debug:  %s", to_mask_str(is_masked<Debug>()), nullptr);
  info("      SError: %s", to_mask_str(is_masked<SError>()), nullptr);
  info("      IRQ:    %s", to_mask_str(is_masked<IRQ>()), nullptr);
  info("      FIQ:    %s", to_mask_str(is_masked<FIQ>()), nullptr);
}

namespace Asynchronous {

extern "C" unsigned long read_daif() {
  unsigned long value;
  asm volatile("mrs %0, daif" : "=r"(value));
  return value;
}

// extern "C" {
unsigned long localIrqMaskSave() {
  unsigned long flags;
  asm volatile("mrs %0, daif\n"
               "msr daifset, #2"
               : "=r"(flags)
               :
               : "memory");
  return flags;
}

void localIrqRestore(unsigned long flags) {
  asm volatile("msr daif, %0" : : "r"(flags) : "memory");
}
void localIrqMask() { asm volatile("msr daifset, #2" ::: "memory"); }

void localIrqUnmask() { asm volatile("msr daifclr, #2" ::: "memory"); }

bool isLocalIrqMasked() {
  unsigned long flags;
  asm volatile("mrs %0, daif" : "=r"(flags));
  return (flags & (1 << 7)) != 0; // IRQ bit in DAIF
}

//} // extern "C"
} // namespace Asynchronous
} // namespace Exception