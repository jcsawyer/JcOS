#pragma once

#include <bsp/exception/asynchronous.hpp>
#include <exceptions/asynchronous.hpp>
#include <stdint.h>
#include <synchronization.hpp>

namespace Driver::BSP::BCM2XXX {

class PeripheralIC : public Exceptions::Asynchronous::IRQManager {
public:
  PeripheralIC(uint32_t mmio_start_addr) : registerBlock(mmio_start_addr) {
    for (int i = 0; i < MAX_IRQS; ++i) {
      handlerTable[i] = nullptr;
    }
  }
  virtual ~PeripheralIC() = default;
  void registerHandler(
      const Exceptions::Asynchronous::IRQHandlerDescriptor &handler) override;

  void enable(::BSP::Exception::Asynchronous::IRQNumber *number) override;
  void
  handlePendingIrqs(const Exceptions::Asynchronous::IRQContext &ctx) override;
  void printHandler() override;

private:
  static constexpr int MAX_IRQS = 64;

  class RegisterBlock {
  public:
    volatile uint32_t *ENABLE_1;
    volatile uint32_t *ENABLE_2;
    volatile uint32_t *PENDING_1;
    volatile uint32_t *PENDING_2;

    RegisterBlock(uint32_t base) {
      PENDING_1 = reinterpret_cast<volatile uint32_t *>(base + 0x04);
      PENDING_2 = reinterpret_cast<volatile uint32_t *>(base + 0x08);
      ENABLE_1 = reinterpret_cast<volatile uint32_t *>(base + 0x10);
      ENABLE_2 = reinterpret_cast<volatile uint32_t *>(base + 0x14);
    }
  };

  RegisterBlock registerBlock;
  Exceptions::Asynchronous::IRQHandlerDescriptor *handlerTable[MAX_IRQS];
};

} // namespace Driver::BSP::BCM2XXX
