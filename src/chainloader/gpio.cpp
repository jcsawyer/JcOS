#include "gpio.hpp"

#include <arch/cpu.hpp>

namespace Chainloader {

GPIO::GPIO(uintptr_t baseAddress) {
  gpfsel0_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x00);
  gpfsel1_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x04);
  gpset0_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x1C);
  gpclr0_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x28);
  gppud_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x94);
  gppudclk0_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x98);
  gpioPupPdnCntrlReg0_ =
      reinterpret_cast<volatile uint32_t *>(baseAddress + 0xE4);
}

void GPIO::configureOutput(unsigned char pin) {
  volatile uint32_t *gpfsel = gpfsel0_ + (pin / 10);
  const unsigned int shift = (pin % 10) * 3;
  uint32_t value = *gpfsel;
  value &= ~(0b111u << shift);
  value |= (0b001u << shift);
  *gpfsel = value;
}

void GPIO::write(unsigned char pin, bool high) {
  volatile uint32_t *reg = high ? gpset0_ : gpclr0_;
  *(reg + (pin / 32)) = (1u << (pin % 32));
}

void GPIO::disablePud1415Bcm2837() const {
  uint32_t value = *gpfsel1_;
  value &= ~static_cast<uint32_t>((7u << 12) | (7u << 15));
  value |= static_cast<uint32_t>((4u << 12) | (4u << 15));
  *gpfsel1_ = value;

  *gppud_ = 0;
  CPU::spinForCycles(150);
  *gppudclk0_ = 0;
}

void GPIO::disablePud1415Bcm2711() const {
  *gpioPupPdnCntrlReg0_ = (0b01u << 30) | (0b01u << 28);
}

void GPIO::mapUartPins() const {
#if BOARD == bsp_rpi3
  disablePud1415Bcm2837();
#elif BOARD == bsp_rpi4
  disablePud1415Bcm2711();
#else
#error Unsupported board configuration
#endif
}

} // namespace Chainloader
