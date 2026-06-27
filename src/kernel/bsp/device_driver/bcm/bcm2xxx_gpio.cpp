#include "bcm2xxx_gpio.hpp"

namespace Driver::BSP::BCM {

void GPIO::init() {}

void GPIO::registerAndEnableIrqHandler(
    ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) {}

void GPIO::configureOutput(unsigned int pin) const {
  setFunction(pin, 0b001u);
}

void GPIO::write(unsigned int pin, bool high) const {
  if (high) {
    *(registerBlock.GPSET0 + (pin / 32)) = (1u << (pin % 32));
  } else {
    *(registerBlock.GPCLR0 + (pin / 32)) = (1u << (pin % 32));
  }
}

void GPIO::configureInput(unsigned int pin) const { setFunction(pin, 0b000u); }

bool GPIO::read(unsigned int pin) const {
  return (*(registerBlock.GPLEV0 + (pin / 32)) & (1u << (pin % 32))) != 0u;
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

void GPIO::mapI2c1() const {
  setAltFunction(2, 0b100);
  setAltFunction(3, 0b100);
  configurePull(2, PullMode::PullUp);
  configurePull(3, PullMode::PullUp);
}

void GPIO::configurePull(unsigned int pin, PullMode mode) const {
#if BOARD == bsp_rpi3
  setPullBcm2837(pin, mode);
#elif BOARD == bsp_rpi4
  setPullBcm2711(pin, mode);
#else
#error Unknown board
#endif
}

void GPIO::configureRisingEdgeDetect(unsigned int pin, bool enable) const {
  volatile unsigned int *reg = registerBlock.GPREN0 + (pin / 32);
  const unsigned int mask = 1u << (pin % 32);
  if (enable) {
    *reg |= mask;
  } else {
    *reg &= ~mask;
  }
}

void GPIO::configureFallingEdgeDetect(unsigned int pin, bool enable) const {
  volatile unsigned int *reg = registerBlock.GPFEN0 + (pin / 32);
  const unsigned int mask = 1u << (pin % 32);
  if (enable) {
    *reg |= mask;
  } else {
    *reg &= ~mask;
  }
}

bool GPIO::eventDetected(unsigned int pin) const {
  return (*(registerBlock.GPEDS0 + (pin / 32)) & (1u << (pin % 32))) != 0u;
}

void GPIO::clearEventDetect(unsigned int pin) const {
  *(registerBlock.GPEDS0 + (pin / 32)) = (1u << (pin % 32));
}

void GPIO::setAltFunction(unsigned int pin, unsigned int altFunction) const {
  setFunction(pin, altFunction & 0b111u);
}

void GPIO::setFunction(unsigned int pin, unsigned int function) const {
  volatile unsigned int *gpfsel = registerBlock.GPFSEL0 + (pin / 10);
  const unsigned int shift = (pin % 10) * 3;
  unsigned int value = *gpfsel;
  value &= ~(0b111u << shift);
  value |= ((function & 0b111u) << shift);
  *gpfsel = value;
}

void GPIO::setPullBcm2837(unsigned int pin, PullMode mode) const {
  uint32_t pud = 0;
  switch (mode) {
  case PullMode::None:
    pud = 0;
    break;
  case PullMode::PullDown:
    pud = 0b01;
    break;
  case PullMode::PullUp:
    pud = 0b10;
    break;
  }

  *registerBlock.GPPUD = pud;
  CPU::spinForCycles(150);
  *(registerBlock.GPPUDCLK0 + (pin / 32)) = (1u << (pin % 32));
  CPU::spinForCycles(150);
  *registerBlock.GPPUD = 0;
  *(registerBlock.GPPUDCLK0 + (pin / 32)) = 0;
}

void GPIO::setPullBcm2711(unsigned int pin, PullMode mode) const {
  const unsigned int shift = (pin % 16) * 2;
  volatile unsigned int *reg =
      registerBlock.GPIO_PUP_PDN_CNTRL_REG0 + (pin / 16);
  uint32_t value = *reg;
  value &= ~(0b11u << shift);

  uint32_t bits = 0;
  switch (mode) {
  case PullMode::None:
    bits = 0b00;
    break;
  case PullMode::PullUp:
    bits = 0b01;
    break;
  case PullMode::PullDown:
    bits = 0b10;
    break;
  }

  value |= (bits << shift);
  *reg = value;
}

} // namespace Driver::BSP::BCM
