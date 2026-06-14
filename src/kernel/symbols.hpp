#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Symbols {

struct KernelSymbol {
  uintptr_t start;
  uintptr_t size;
  const char *name;

  bool contains(uintptr_t address) const {
    return address >= start && address < (start + size);
  }
};

const KernelSymbol *lookupSymbol(uintptr_t address);

extern "C" {
extern size_t __kernel_symbols_start;
extern size_t __kernel_symbols_end_exclusive;
extern volatile uint64_t NUM_KERNEL_SYMBOLS;
}

} // namespace Symbols
