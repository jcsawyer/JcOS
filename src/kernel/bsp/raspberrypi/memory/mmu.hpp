#pragma once

#include "../../../memory/mmu.hpp"
#include "../memory.hpp"

namespace Memory {
using KernelAddrSpace = AddressSpace<Memory::Map::END_INCLUSIVE + 1>;

TranslationDescriptor *translationsDescriptors();
} // namespace Memory
