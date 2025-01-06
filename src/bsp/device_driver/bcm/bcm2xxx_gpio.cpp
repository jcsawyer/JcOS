#include "bcm2xxx_gpio.hpp"
#include "../../../_arch/aarch64/aarch_cpu.hpp"

namespace Driver::BSP::BCM {

void GPIO::init() {}

void GPIO::disablePud1415Bcm2837() {

  uint32_t r = 0;
  // Configure GPIO 14 and 15
  r = *registerBlock.GPFSEL1;
  r &= ~(static_cast<uint32_t>((7 << 12) | (7 << 15))); // Clear GPIO 14, 15
  r |= (4 << 12) | (4 << 15); // Set alt0 for GPIO 14, 15
  *registerBlock.GPFSEL1 = r;
  *registerBlock.GPPUD = 0; // Enable pins 14 and 15
  spinForCycles(150);
  *registerBlock.GPPUDCLK0 = 0; // Flush GPIO setup
}

void GPIO::disablePud1415Bcm2711() {
  *registerBlock.GPIO_PUP_PDN_CNTRL_REG0 = (1 << 30) | (1 << 28);
}

void GPIO::mapPl011Uart() {
#if BOARD == bsp_rpi3
  disablePud1415Bcm2837();
#elif BOARD == bsp_rpi4
  disablePud1415Bcm2711();
#else
#error Unknown board
#endif
}

} // namespace Driver::BSP::BCM
