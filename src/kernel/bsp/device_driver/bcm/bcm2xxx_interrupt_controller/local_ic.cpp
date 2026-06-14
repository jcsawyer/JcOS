#include "local_ic.hpp"
#include <panic.hpp>

namespace Driver::BSP::BCM2XXX {

using IRQHandlerDescriptor = Exceptions::Asynchronous::IRQHandlerDescriptor;

void LocalIC::init() { handlerTable.resize(MAX_IRQS, nullptr); }

void LocalIC::registerHandler(const IRQHandlerDescriptor &handler) {
  unsigned long irq_num = handler.number.local.get();

  if (irq_num >= MAX_IRQS) {
    panic("LocalIC: IRQ number out of range");
  }

  if (handlerTable[irq_num] != nullptr) {
    panic("LocalIC: IRQ handler already registered");
  }

  handlerTable[irq_num] = new IRQHandlerDescriptor(handler);
}

void LocalIC::enable(::BSP::Exception::Asynchronous::IRQNumber *number) {
  *registerBlock.CORE0_TIMER_INTERRUPT_CONTROL = 1u << number->local.get();
}

void LocalIC::handlePendingIrqs(const Exceptions::Asynchronous::IRQContext &) {
  const uint32_t pending =
      *registerBlock.CORE0_INTERRUPT_SOURCE & ~PERIPH_IRQ_MASK;

  for (uint32_t irq = 0; irq < MAX_IRQS; ++irq) {
    if ((pending & (1u << irq)) == 0) {
      continue;
    }

    IRQHandlerDescriptor *handler = handlerTable[irq];
    if (handler == nullptr) {
      panic("LocalIC: No handler registered for IRQ ", irq);
    }

    handler->handler->handle();
  }
}

void LocalIC::printHandler() {
  info("      Local handler:");

  for (uint32_t i = 0; i < MAX_IRQS; ++i) {
    if (handlerTable[i] != nullptr) {
      info("            %d. %s", i, handlerTable[i]->name);
    }
  }
}

} // namespace Driver::BSP::BCM2XXX
