#include "bcm2xxx_rng.hpp"

namespace Driver {
namespace BSP {
namespace BCM {
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

unsigned int RNG::next(unsigned int min, unsigned int max) {
  // Check if RNG is enabled
  if (!(*registerBlock.RNG_CTRL & 1)) {
    return -1;
  }

  // Wait for RNG to be entropy ready
  while (!*registerBlock.RNG_STATUS >> 24) {
    CPU::nop();
  }

  // Get random number
  int rand = *registerBlock.RNG_DATA;
  // Scale random number to be within range
  return (rand % (max - min)) + min;
}
} // namespace BCM
} // namespace BSP
} // namespace Driver