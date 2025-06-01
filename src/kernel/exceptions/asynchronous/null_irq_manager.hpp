#pragma once
#include <exceptions/asynchronous.hpp>
#include <panic.hpp>

namespace Exceptions {
namespace Asynchronous {

class NullIRQManager : public IRQManager {
public:
  void registerHandler(const Exceptions::Asynchronous::IRQHandlerDescriptor
                           &irq_handler_descriptor) override {
    panic("Register Handler: No IRQ Manager registered yet");
  }
  void enable(::BSP::Exception::Asynchronous::IRQNumber *number) override {
    panic("Enable: No IRQ Manager registered yet");
  }
  void handlePendingIrqs(const IRQContext &ctx) override {
    panic("Handle Pending IRQs: No IRQ Manager registered yet");
  }
  void printHandler() override {
    panic("Print Handler: No IRQ Manager registered yet");
  }
};

NullIRQManager null_irq_manager;
} // namespace Asynchronous
} // namespace Exceptions