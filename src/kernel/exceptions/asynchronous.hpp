#pragma once

#include <arch/exception.hpp>
#include <bsp/exception/asynchronous.hpp>

namespace Exceptions {
namespace Asynchronous {

struct IRQContext;

class IRQHandler {
public:
  virtual bool handle() = 0;
};

struct IRQHandlerDescriptor {
  BSP::Exception::Asynchronous::IRQKind kind;
  BSP::Exception::Asynchronous::IRQNumber number;
  const char *name;
  IRQHandler *handler;

  IRQHandlerDescriptor()
      : kind(BSP::Exception::Asynchronous::IRQKind::Peripheral),
        number(BSP::Exception::Asynchronous::IRQNumber{
            BSP::Exception::Asynchronous::IRQKind::Peripheral,
            BSP::Exception::Asynchronous::PeripheralIRQ::newBounded(-1)
                .value()}),
        name(nullptr), handler(nullptr) {}

  IRQHandlerDescriptor(BSP::Exception::Asynchronous::IRQKind k,
                       BSP::Exception::Asynchronous::IRQNumber num,
                       const char *nm, IRQHandler *hnd)
      : kind(k), number(num), name(nm), handler(hnd) {}
};

struct IRQContext {
public:
  IRQContext create();

private:
  IRQContext() = default;
  IRQContext(const IRQContext &) = default;
};

class IRQManager {
public:
  virtual void registerHandler(
      const Exceptions::Asynchronous::IRQHandlerDescriptor &handler) = 0;
  virtual void enable(BSP::Exception::Asynchronous::IRQNumber *number) = 0;
  virtual void handlePendingIrqs(const IRQContext &ctx) = 0;
  virtual void printHandler() = 0;
};

class IRQManagerHolder {
private:
  static IRQManager *manager;

public:
  static void registerManager(IRQManager *m) { manager = m; }

  static IRQManager *get() { return manager; }
};

IRQManager *irq_manager();

void registerIrqManager(IRQManager *newManager);

template <typename Func> inline void execWithIrqMasked(Func &&func) {
  unsigned int saved = Exception::Asynchronous::localIrqMaskSave();
  func();
  Exception::Asynchronous::localIrqRestore(saved);
}

} // namespace Asynchronous
} // namespace Exceptions
