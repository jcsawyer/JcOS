#include "peripheral_ic.hpp"
#include <bsp/device_driver/common.hpp>
#include <bsp/exception/asynchronous.hpp>
#include <panic.hpp>

namespace Driver::BSP::BCM2XXX {

using IRQHandlerDescriptor = Exceptions::Asynchronous::IRQHandlerDescriptor;
using IRQNumber = ::BSP::Exception::Asynchronous::IRQNumber;

void PeripheralIC::registerHandler(const IRQHandlerDescriptor &handler) {
  unsigned long irq_num = handler.number.peripheral.get();

  if (irq_num >= MAX_IRQS) {
    panic("PeripheralIC: IRQ number out of range");
  }

  if (handlerTable[irq_num] != nullptr) {
    panic("PeripheralIC: IRQ handler already registered");
  }

  handlerTable[irq_num] = const_cast<IRQHandlerDescriptor *>(&handler);
}

void PeripheralIC::enable(IRQNumber *number) {
  uint32_t irq = number->peripheral.get();
  uint32_t bit = 1u << (irq % 32);

  if (irq < 32) {
    *registerBlock.ENABLE_1 = bit;
  } else {
    *registerBlock.ENABLE_2 = bit;
  }
}

void PeripheralIC::handlePendingIrqs(
    const Exceptions::Asynchronous::IRQContext &) {
  uint64_t pending_mask =
      (static_cast<uint64_t>(*registerBlock.PENDING_2) << 32) |
      static_cast<uint64_t>(*registerBlock.PENDING_1);

  for (uint32_t irq = 0; irq < MAX_IRQS; ++irq) {
    if (pending_mask & (1ull << irq)) {
      IRQHandlerDescriptor *handler = handlerTable[irq];

      if (handler == nullptr) {
        panic("PeripheralIC: No handler registered for IRQ ", irq);
      }

      handler->handler->handle();
    }
  }
}

void PeripheralIC::printHandler() {
  info("      Peripheral handler:");

  for (uint32_t i = 0; i < MAX_IRQS; ++i) {
    if (handlerTable[i] != nullptr) {
      info("            %d. %s", i, handlerTable[i]->name);
    }
  }
}

} // namespace Driver::BSP::BCM2XXX