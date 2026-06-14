#include <symbols.hpp>

namespace Symbols {

extern "C" {
volatile uint64_t NUM_KERNEL_SYMBOLS __attribute__((section(".data"))) = 0;
}

namespace {
const KernelSymbol *kernelSymbolsStart() {
  return reinterpret_cast<const KernelSymbol *>(&__kernel_symbols_start);
}

size_t numKernelSymbols() { return static_cast<size_t>(NUM_KERNEL_SYMBOLS); }
} // namespace

const KernelSymbol *lookupSymbol(uintptr_t address) {
  const KernelSymbol *symbols = kernelSymbolsStart();
  size_t count = numKernelSymbols();

  for (size_t i = 0; i < count; ++i) {
    if (symbols[i].contains(address)) {
      return &symbols[i];
    }
  }

  return nullptr;
}

} // namespace Symbols
