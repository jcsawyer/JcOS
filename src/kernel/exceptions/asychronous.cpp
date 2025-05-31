#include "asynchronous.hpp"
#include <exceptions/asynchronous/null_irq_manager.hpp>

namespace Exceptions {
namespace Asynchronous {
// Global manager instance storage
static IRQManager *g_irqManager = &null_irq_manager;

void registerIrqManager(IRQManager *newManager) { g_irqManager = newManager; }

IRQManager *irq_manager() { return g_irqManager; }

IRQManager *IRQManagerHolder::manager = nullptr;

IRQContext IRQContext::create() {
  // NOTE: Must only be called from IRQ context.
  return IRQContext{};
}

// Executes the provided function with IRQs masked
void exec_with_irq_masked(void (*fn)(void)) {
  unsigned long saved = Exception::Asynchronous::localIrqMaskSave();
  fn();
  Exception::Asynchronous::localIrqRestore(saved);
}

} // namespace Asynchronous
} // namespace Exceptions