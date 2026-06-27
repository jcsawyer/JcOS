#pragma once

#include "../../../arch/cpu.hpp"
#include "../../../driver/driver.hpp"
#include <bsp/exception/asynchronous.hpp>
#include <stdint.h>

namespace Driver {
namespace BSP {
namespace BCM {
class GPIO : public Driver::DeviceDriver {
public:
  GPIO(uintptr_t mmio_start_addr) : registerBlock(mmio_start_addr) {};
  const char *compatible() override { return "BCM GPIO"; }
  void init() override;
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;
  enum class PullMode {
    None,
    PullUp,
    PullDown,
  };
  void configureOutput(unsigned int pin) const;
  void configureInput(unsigned int pin) const;
  void write(unsigned int pin, bool high) const;
  bool read(unsigned int pin) const;
  void configurePull(unsigned int pin, PullMode mode) const;
  void mapPl011Uart() const;
  void mapSpi0() const;
  void mapI2c1() const;
  void configureRisingEdgeDetect(unsigned int pin, bool enable) const;
  void configureFallingEdgeDetect(unsigned int pin, bool enable) const;
  bool eventDetected(unsigned int pin) const;
  void clearEventDetect(unsigned int pin) const;

private:
  class RegisterBlock {
  public:
    volatile unsigned int *GPFSEL0;
    volatile unsigned int *GPFSEL1;
    volatile unsigned int *GPSET0;
    volatile unsigned int *GPCLR0;
    volatile unsigned int *GPLEV0;
    volatile unsigned int *GPEDS0;
    volatile unsigned int *GPREN0;
    volatile unsigned int *GPFEN0;
    volatile unsigned int *GPPUD;
    volatile unsigned int *GPPUDCLK0;
    volatile unsigned int *GPIO_PUP_PDN_CNTRL_REG0;

    RegisterBlock(uintptr_t mmio_start_addr) {
      GPFSEL0 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x00);
      GPFSEL1 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x04);
      GPSET0 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x1C);
      GPCLR0 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x28);
      GPLEV0 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x34);
      GPEDS0 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x40);
      GPREN0 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x4C);
      GPFEN0 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x58);
      GPPUD = reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x94);
      GPPUDCLK0 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x98);
      GPIO_PUP_PDN_CNTRL_REG0 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0xE4);
    }
  };
  RegisterBlock registerBlock;
  void disablePud1415Bcm2837() const;
  void disablePud1415Bcm2711() const;
  void setPullBcm2837(unsigned int pin, PullMode mode) const;
  void setPullBcm2711(unsigned int pin, PullMode mode) const;
  void setAltFunction(unsigned int pin, unsigned int altFunction) const;
  void setFunction(unsigned int pin, unsigned int function) const;
};
} // namespace BCM
} // namespace BSP
} // namespace Driver
