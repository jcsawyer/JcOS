#pragma once

#include <stdint.h>

namespace Chainloader {

class GPIO {
public:
  explicit GPIO(uintptr_t baseAddress);

  void configureOutput(unsigned char pin);
  void write(unsigned char pin, bool high);
  void mapUartPins() const;

private:
  volatile uint32_t *gpfsel0_;
  volatile uint32_t *gpfsel1_;
  volatile uint32_t *gpset0_;
  volatile uint32_t *gpclr0_;
  volatile uint32_t *gppud_;
  volatile uint32_t *gppudclk0_;
  volatile uint32_t *gpioPupPdnCntrlReg0_;

  void disablePud1415Bcm2837() const;
  void disablePud1415Bcm2711() const;
};

} // namespace Chainloader
