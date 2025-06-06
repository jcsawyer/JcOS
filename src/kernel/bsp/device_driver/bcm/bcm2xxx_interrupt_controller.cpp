#include "bcm2xxx_interrupt_controller.hpp"

namespace Driver::BSP::BCM {

void InterruptController::registerHandler(
    const Exceptions::Asynchronous::IRQHandlerDescriptor
        &irq_handler_descriptor) {
  switch (irq_handler_descriptor.number.kind) {
  case ::BSP::Exception::Asynchronous::IRQKind::Local:
    panic("Local IRQs not supported in this BSP.");

  case ::BSP::Exception::Asynchronous::IRQKind::Peripheral: {
    auto pirq = irq_handler_descriptor.number;

    static Exceptions::Asynchronous::IRQHandlerDescriptor periph_descriptor{
        ::BSP::Exception::Asynchronous::IRQKind::Peripheral, pirq,
        irq_handler_descriptor.name, irq_handler_descriptor.handler};

    periph.registerHandler(periph_descriptor);
  }
  }
}

} // namespace Driver::BSP::BCM