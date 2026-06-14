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

void GPIO::mapSpi0() const {
  for (unsigned int pin = 7; pin <= 11; ++pin) {
    setFunction(pin, GPIOFunction::Alt0);
  }
}

void GPIO::setFunction(unsigned int pin, GPIOFunction function) const {
  volatile unsigned int *gpfsel =
      registerBlock.GPFSEL0 + (pin / 10); // each GPFSEL controls 10 pins
  const int shift = (pin % 10) * 3;
  unsigned int value = *gpfsel;
  value &= ~(0b111u << shift);
  value |= (static_cast<unsigned int>(function) << shift);
  *gpfsel = value;
}

void GPIO::setOutput(unsigned int pin) const {
  setFunction(pin, GPIOFunction::Output);
}

void GPIO::write(unsigned int pin, bool high) const {
  if (high) {
    *(registerBlock.GPSET + (pin / 32)) = (1u << (pin % 32));
    return;
  }

  *(registerBlock.GPCLR + (pin / 32)) = (1u << (pin % 32));
}

} // namespace Driver::BSP::BCM
