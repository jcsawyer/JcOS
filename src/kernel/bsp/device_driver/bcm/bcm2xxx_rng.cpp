#include "bcm2xxx_rng.hpp"

namespace Driver::BSP::BCM {

void RNG::init() {
  *registerBlock.RNG_STATUS = 0x40000;
  // Mask interrupt
  *registerBlock.RNG_INT_MASK |= 1;
  // Enable RNG
  *registerBlock.RNG_CTRL = 1;
  // Wait for RNG to be entropy ready
  while (!*registerBlock.RNG_STATUS >> 24) {
    CPU::nop();
  }
}

void RNG::registerAndEnableIrqHandler() {}

unsigned int RNG::next(const unsigned int min, const unsigned int max) const {
  // Check if RNG is enabled
  if (!(*registerBlock.RNG_CTRL & 1)) {
    return -1;
  }

  // Wait for RNG to be entropy ready
  while (!*registerBlock.RNG_STATUS >> 24) {
    CPU::nop();
  }

  // Get random number
  const int rand = *registerBlock.RNG_DATA;
  // Scale random number to be within range
  return rand % (max - min) + min;
}
} // namespace Driver::BSP::BCM
