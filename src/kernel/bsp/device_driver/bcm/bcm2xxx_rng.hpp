#pragma once

#include "../../../arch/cpu.hpp"
#include "../../../driver/driver.hpp"
#include <bsp/exception/asynchronous.hpp>
#include <stdint.h>

namespace Driver {
namespace BSP {
namespace BCM {
class RNG : public Driver::DeviceDriver {
public:
  RNG(unsigned int mmio_start_addr) : registerBlock(mmio_start_addr) {};
  const char *compatible() override { return "BCM RNG"; }
  void init() override;
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;
  unsigned int next(unsigned int min, unsigned int max) const;

private:
  class RegisterBlock {
  public:
    volatile unsigned int *RNG_CTRL;
    volatile unsigned int *RNG_STATUS;
    volatile unsigned int *RNG_DATA;
    volatile unsigned int *RNG_INT_MASK;

    RegisterBlock(unsigned int mmio_start_addr) {
      RNG_CTRL =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x00);
      RNG_STATUS =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x04);
      RNG_DATA =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x08);
      RNG_INT_MASK =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x10);
    }
  };
  RegisterBlock registerBlock;
};
} // namespace BCM
} // namespace BSP
} // namespace Driver
