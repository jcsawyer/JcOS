#pragma once

#include <bsp/exception/asynchronous.hpp>
#include <container/vector.hpp>
#include <exceptions/asynchronous.hpp>
#include <stdint.h>

namespace Driver::BSP::BCM2XXX {

class LocalIC : public Exceptions::Asynchronous::IRQManager {
public:
  explicit LocalIC(uintptr_t mmio_start_addr)
      : registerBlock(mmio_start_addr) {}

  void init();
  void registerHandler(
      const Exceptions::Asynchronous::IRQHandlerDescriptor &handler) override;
  void enable(::BSP::Exception::Asynchronous::IRQNumber *number) override;
  void
  handlePendingIrqs(const Exceptions::Asynchronous::IRQContext &ctx) override;
  void printHandler() override;

private:
  static constexpr int MAX_IRQS = 4;
  static constexpr uint32_t PERIPH_IRQ_MASK = (1 << 8);

  class RegisterBlock {
  public:
    volatile uint32_t *CORE0_TIMER_INTERRUPT_CONTROL;
    volatile uint32_t *CORE0_INTERRUPT_SOURCE;

    explicit RegisterBlock(uintptr_t base) {
      CORE0_TIMER_INTERRUPT_CONTROL =
          reinterpret_cast<volatile uint32_t *>(base + 0x40);
      CORE0_INTERRUPT_SOURCE =
          reinterpret_cast<volatile uint32_t *>(base + 0x60);
    }
  };

  RegisterBlock registerBlock;
  Container::Vector<Exceptions::Asynchronous::IRQHandlerDescriptor *>
      handlerTable;
};

} // namespace Driver::BSP::BCM2XXX
