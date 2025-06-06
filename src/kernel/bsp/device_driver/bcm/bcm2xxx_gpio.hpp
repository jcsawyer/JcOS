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
  GPIO(unsigned int mmio_start_addr) : registerBlock(mmio_start_addr) {};
  const char *compatible() override { return "BCM GPIO"; }
  void init() override;
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;
  void mapPl011Uart() const;

private:
  class RegisterBlock {
  public:
    volatile unsigned int *GPFSEL1;
    volatile unsigned int *GPPUD;
    volatile unsigned int *GPPUDCLK0;
    volatile unsigned int *GPIO_PUP_PDN_CNTRL_REG0;

    RegisterBlock(unsigned int mmio_start_addr) {
      GPFSEL1 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x04);
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
};
} // namespace BCM
} // namespace BSP
} // namespace Driver
