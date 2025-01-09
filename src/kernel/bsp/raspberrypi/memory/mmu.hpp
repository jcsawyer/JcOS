#pragma once

#include "../../../memory/mmu.hpp"
#include "../memory.hpp"

namespace Memory
{
    using KernelAddrSpace = AddressSpace<Memory::Map::END_INCLUSIVE + 1>;

    const size_t NUM_MEM_RANGES = 3;

    TranslationDescriptor* translationsDescriptors();
    KernelVirtualLayout<NUM_MEM_RANGES> *virtMemLayout();
} // namespace Memory
