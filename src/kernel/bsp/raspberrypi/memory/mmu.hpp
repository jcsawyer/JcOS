#pragma once

#include "../../../memory/mmu.hpp"
#include "../memory.hpp"

namespace Memory {
using KernelAddrSpace = AddressSpace<Memory::Map::KERNEL_VIRT_ADDR_SPACE_SIZE>;
} // namespace Memory
