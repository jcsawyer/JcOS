#include "bcm2xxx_interrupt_controller.hpp"

namespace Driver::BSP::BCM {

void InterruptController::init() {
  local.init();
  periph.init();
}

void InterruptController::registerHandler(
    const Exceptions::Asynchronous::IRQHandlerDescriptor
        &irq_handler_descriptor) {
  switch (irq_handler_descriptor.number.kind) {
  case ::BSP::Exception::Asynchronous::IRQKind::Local:
    local.registerHandler(irq_handler_descriptor);
    break;

  case ::BSP::Exception::Asynchronous::IRQKind::Peripheral:
    periph.registerHandler(irq_handler_descriptor);
    break;
  }
}

} // namespace Driver::BSP::BCM
