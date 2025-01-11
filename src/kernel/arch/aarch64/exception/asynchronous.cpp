#include "asynchronous.hpp"
#include "../../../print.hpp"

namespace Exception {

bool is_set(uint64_t field) {
  uint64_t daif_value;
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

extern "C" uint64_t read_daif() {
  uint64_t value;
  asm volatile("mrs %0, daif" : "=r"(value));
  return value;
}
} // namespace Exception
