#pragma once

#include <arch/exception.hpp>

namespace Exceptions {
namespace Asynchronous {

struct IRQContext;

class IRQHandler {
public:
  virtual bool handle() = 0;
};

class IRQManager {
public:
  virtual void registerHandler(unsigned int number, const char *name,
                               IRQHandler *handler) = 0;
  virtual void enable(unsigned int number) = 0;
  virtual void handlePendingIrqs(const IRQContext &ctx) = 0;
  virtual void printHandler() {}
};

struct IRQHandlerDescriptor {
  unsigned int number;
  const char *name;
  IRQHandler *handler;

  IRQHandlerDescriptor(unsigned int num, const char *nm, IRQHandler *hnd)
      : number(num), name(nm), handler(hnd) {}
};

struct IRQContext {
public:
  IRQContext create();

private:
  IRQContext() = default;
  IRQContext(const IRQContext &) = default;
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

inline void execWithIrqMasked(void (*func)()) {
  unsigned int saved = Exception::Asynchronous::localIrqMaskSave();
  func();
  Exception::Asynchronous::localIrqRestore(saved);
}

} // namespace Asynchronous
} // namespace Exceptions
