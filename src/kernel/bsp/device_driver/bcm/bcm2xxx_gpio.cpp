#include "bcm2xxx_gpio.hpp"

namespace Driver::BSP::BCM {

void GPIO::init() {}

void GPIO::registerAndEnableIrqHandler(
    ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) {}

void GPIO::disablePud1415Bcm2837() const {

  uint32_t r = 0;
  // Configure GPIO 14 and 15
  r = *registerBlock.GPFSEL1;
  r &= ~static_cast<uint32_t>(7 << 12 | 7 << 15); // Clear GPIO 14, 15
  r |= 4 << 12 | 4 << 15;                         // Set alt0 for GPIO 14, 15
  *registerBlock.GPFSEL1 = r;
  *registerBlock.GPPUD = 0; // Enable pins 14 and 15
  CPU::spinForCycles(150);
  *registerBlock.GPPUDCLK0 = 0; // Flush GPIO setup
}

void GPIO::disablePud1415Bcm2711() const {
  *registerBlock.GPIO_PUP_PDN_CNTRL_REG0 = 0b01 << 30 | 0b01 << 28;
}

void GPIO::mapPl011Uart() const {
#if BOARD == bsp_rpi3
  disablePud1415Bcm2837();
#elif BOARD == bsp_rpi4
  disablePud1415Bcm2711();
#else
#error Unknown board
#endif
}

} // namespace Driver::BSP::BCM
