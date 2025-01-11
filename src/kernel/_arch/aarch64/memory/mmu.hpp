#pragma once

#include "mmu/translation_table.hpp"
#include <memory/mmu.hpp>
#include <stddef.h>
#include <stdint.h>

namespace Memory {
// Memory Management Unit type
class MemoryManagementUnit {
public:
  static void setUpMAIR();
  static void configureTranslationControl();

  static bool isEnabled();

  static void enableMMUAndCaching();

  enum MAIR : uint64_t {
    DEVICE = 0,
    NORMAL = 1,
  };
};

// TODO : Move back into mmu.cpp
KernelTranslationTable *kernelTables();

MemoryManagementUnit *MMU();

} // namespace Memory
