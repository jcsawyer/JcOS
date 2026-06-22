#pragma once

#include "../../../arch/cpu.hpp"
#include "../../../driver/driver.hpp"
#include <bsp/exception/asynchronous.hpp>
#include <stdint.h>

namespace Driver {
namespace BSP {
namespace BCM {

enum class GPIOFunction : uint32_t {
  Input = 0b000,
  Output = 0b001,
  Alt0 = 0b100,
  Alt1 = 0b101,
  Alt2 = 0b110,
  Alt3 = 0b111,
  Alt4 = 0b011,
  Alt5 = 0b010,
};

class GPIO : public Driver::DeviceDriver {
public:
  GPIO(uintptr_t mmio_start_addr) : registerBlock(mmio_start_addr) {};
  const char *compatible() override { return "BCM GPIO"; }
  void init() override;
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;
  void mapPl011Uart() const;
  void mapSpi0() const;
  void setFunction(unsigned int pin, GPIOFunction function) const;
  void setOutput(unsigned int pin) const;
  void write(unsigned int pin, bool high) const;

private:
  class RegisterBlock {
  public:
    volatile unsigned int *GPFSEL0;
    volatile unsigned int *GPFSEL1;
    volatile unsigned int *GPFSEL2;
    volatile unsigned int *GPSET;
    volatile unsigned int *GPCLR;
    volatile unsigned int *GPPUD;
    volatile unsigned int *GPPUDCLK0;
    volatile unsigned int *GPIO_PUP_PDN_CNTRL_REG0;

    RegisterBlock(uintptr_t mmio_start_addr) {
      GPFSEL0 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x00);
      GPFSEL1 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x04);
      GPFSEL2 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x08);
      GPSET = reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x1C);
      GPCLR = reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x28);
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
