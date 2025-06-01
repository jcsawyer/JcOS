#pragma once

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
  explicit InterruptController(uintptr_t peripheral_mmio_start)
      : periph(peripheral_mmio_start) {}

  const char *compatible() override { return "BCM Interrupt Controller"; }

  void init() override {
    // No-op for now
  }

  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override {}

  void registerHandler(const Exceptions::Asynchronous::IRQHandlerDescriptor
                           &irq_handler_descriptor) override;

  void enable(::BSP::Exception::Asynchronous::IRQNumber *irq) {
    switch (irq->kind) {
    case ::BSP::Exception::Asynchronous::IRQKind::Local:
      panic("Local IRQs not supported in this BSP.");
      break;

    case ::BSP::Exception::Asynchronous::IRQKind::Peripheral:
      periph.enable(irq);
      break;
    }
  }

  void
  handlePendingIrqs(const Exceptions::Asynchronous::IRQContext &ctx) override {
    periph.handlePendingIrqs(ctx);
  }

  void printHandler() override { periph.printHandler(); }

private:
  Driver::BSP::BCM2XXX::PeripheralIC periph;
};
} // namespace Driver::BSP::BCM