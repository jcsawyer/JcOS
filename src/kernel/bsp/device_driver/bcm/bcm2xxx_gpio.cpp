#include "bcm2xxx_gpio.hpp"

namespace Driver::BSP::BCM {

void GPIO::init() {}

void GPIO::registerAndEnableIrqHandler(
    ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) {}

void GPIO::configureOutput(unsigned int pin) const {
  volatile unsigned int *gpfsel = registerBlock.GPFSEL0 + (pin / 10);
  const unsigned int shift = (pin % 10) * 3;
  unsigned int value = *gpfsel;
  value &= ~(0b111u << shift);
  value |= (0b001u << shift);
  *gpfsel = value;
}

void GPIO::write(unsigned int pin, bool high) const {
  if (high) {
    *(registerBlock.GPSET0 + (pin / 32)) = (1u << (pin % 32));
  } else {
    *(registerBlock.GPCLR0 + (pin / 32)) = (1u << (pin % 32));
  }
}

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

void GPIO::mapSpi0() const {
  setAltFunction(8, 0b100);
  setAltFunction(9, 0b100);
  setAltFunction(10, 0b100);
  setAltFunction(11, 0b100);
}

void GPIO::setAltFunction(unsigned int pin, unsigned int altFunction) const {
  volatile unsigned int *gpfsel = registerBlock.GPFSEL0 + (pin / 10);
  const unsigned int shift = (pin % 10) * 3;
  unsigned int value = *gpfsel;
  value &= ~(0b111u << shift);
  value |= ((altFunction & 0b111u) << shift);
  *gpfsel = value;
}

} // namespace Driver::BSP::BCM
