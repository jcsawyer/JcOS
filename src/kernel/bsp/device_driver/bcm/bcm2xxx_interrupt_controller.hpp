#pragma once

#include "bcm2xxx_interrupt_controller/local_ic.hpp"
#include <bsp/device_driver/bcm/bcm2xxx_interrupt_controller/peripheral_ic.hpp>
#include <bsp/device_driver/common.hpp>
#include <bsp/exception/asynchronous.hpp>
#include <driver/driver.hpp>
#include <exceptions/asynchronous.hpp>
#include <stdint.h>

namespace Driver::BSP::BCM {

class InterruptController : public Driver::DeviceDriver,
                            public Exceptions::Asynchronous::IRQManager {
public:
  InterruptController(uintptr_t local_mmio_start,
                      uintptr_t peripheral_mmio_start)
      : local(local_mmio_start), periph(peripheral_mmio_start) {}

  const char *compatible() override { return "BCM Interrupt Controller"; }

  void init() override;

  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override {}

  void registerHandler(const Exceptions::Asynchronous::IRQHandlerDescriptor
                           &irq_handler_descriptor) override;

  void enable(::BSP::Exception::Asynchronous::IRQNumber *irq) {
    switch (irq->kind) {
    case ::BSP::Exception::Asynchronous::IRQKind::Local:
      local.enable(irq);
      break;

    case ::BSP::Exception::Asynchronous::IRQKind::Peripheral:
      periph.enable(irq);
      break;
    }
  }

  void
  handlePendingIrqs(const Exceptions::Asynchronous::IRQContext &ctx) override {
    local.handlePendingIrqs(ctx);
    periph.handlePendingIrqs(ctx);
  }

  void printHandler() override {
    local.printHandler();
    periph.printHandler();
  }

private:
  Driver::BSP::BCM2XXX::LocalIC local;
  Driver::BSP::BCM2XXX::PeripheralIC periph;
};
} // namespace Driver::BSP::BCM
