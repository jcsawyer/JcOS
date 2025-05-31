#pragma once
#include <exceptions/asynchronous.hpp>
#include <panic.hpp>

namespace Exceptions {
namespace Asynchronous {

class NullIRQManager : public IRQManager {
public:
  void registerHandler(unsigned int number, const char *name,
                       IRQHandler *handler) override {
    panic("No IRQ Maanger registered yet");
  }
  void enable(unsigned int number) override {
    panic("No IRQ Maanger registered yet");
  }
  void handlePendingIrqs(const IRQContext &ctx) override {
    panic("No IRQ Maanger registered yet");
  }
  void printHandler() override { panic("No IRQ Maanger registered yet"); }
};

NullIRQManager null_irq_manager;
} // namespace Asynchronous
} // namespace Exceptions