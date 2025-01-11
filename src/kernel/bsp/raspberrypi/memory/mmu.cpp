#include "mmu.hpp"
#include "../../../memory/mmu.hpp"
#include "../memory.hpp"

namespace Memory {
// Function to return the array
TranslationDescriptor *getDescriptors() {
  static TranslationDescriptor descriptors[NUM_MEM_RANGES] = {
      {
          "Kernel code and RO data",
          codeStart(),
          codeEndExclusive() - 1,
          TranslationType::Identity(),
          {CacheableDRAM, ReadOnly, false},
      },
      {
          "Remapped Device MMIO",
          0x1FFF0000,
          0x1FFFFFFF,
          TranslationType::Offset(Map::getMMIO().START + 0x200000),
          {Device, ReadWrite, true},
      },
      {
          "Device MMIO",
          Map::getMMIO().START,
          Map::getMMIO().END_INCLUSIVE,
          TranslationType::Identity(),
          {Device, ReadWrite, true},
      },
  };

  return descriptors;
}

static KernelVirtualLayout<NUM_MEM_RANGES> *kernelVirtualLayout = nullptr;

KernelVirtualLayout<NUM_MEM_RANGES> *virtMemLayout() {
  if (kernelVirtualLayout == nullptr) {
    static auto kvl = KernelVirtualLayout<NUM_MEM_RANGES>(Map::END_INCLUSIVE,
                                                          getDescriptors());
    kernelVirtualLayout = &kvl;
  }

  return kernelVirtualLayout;
}
} // namespace Memory
