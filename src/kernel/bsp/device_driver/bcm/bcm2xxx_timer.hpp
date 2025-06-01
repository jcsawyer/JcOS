#pragma once

#include <bsp/exception/asynchronous.hpp>
#include <driver/driver.hpp>
#include <stdint.h>

namespace Driver {
namespace BSP {
namespace BCM {

#define TIMER_IRQ_1 (1 << 1)

class Timer : public Driver::DeviceDriver {
public:
  Timer(unsigned int mmio_start_addr) : registerBlock(mmio_start_addr) {};
  const char *compatible() override { return "BCM Timer"; }
  void init() override;
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;
  void timerInit();

private:
  class RegisterBlock {
  public:
    volatile unsigned int *TIMER_CS;
    volatile unsigned int *TIMER_CLO;
    volatile unsigned int *TIMER_C1;

    RegisterBlock(unsigned int mmio_start_addr) {
      TIMER_CS =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x00);
      TIMER_CLO =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x04);
      TIMER_C1 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x10);
    }
  };
  RegisterBlock registerBlock;
};
} // namespace BCM
} // namespace BSP
} // namespace Driver
