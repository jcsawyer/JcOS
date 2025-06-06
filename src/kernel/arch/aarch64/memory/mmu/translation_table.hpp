#pragma once

#include "../../../../bsp/raspberrypi/memory/mmu.hpp"
#include "../../../../memory/mmu/translation_table.hpp"
#include "../mmu.hpp"
#include <panic.hpp>
#include <stddef.h>
#include <stdint.h>

namespace Memory {

using Granule512MiB = TranslationGranule<GRANULE_512M>;
using Granule64KiB = TranslationGranule<GRANULE_64K>;

const size_t NUM_LVL2_TABLES = KernelAddrSpace::size >> Granule512MiB::shift;

template <size_t NUM_TABLES> struct FixedSizeTranslationTable {
  PageDescriptor lvl3[NUM_TABLES][8192];
  TableDescriptor lvl2[NUM_TABLES];

  FixedSizeTranslationTable() : lvl3(), lvl2() {}

  void populateTTEntries() {
    for (uint64_t l2Idx = 0; l2Idx < NUM_TABLES; ++l2Idx) {
      lvl2[l2Idx] =
          TableDescriptor::fromNextLevelTableAddr((uint64_t)&lvl3[l2Idx]);

      for (uint64_t l3Idx = 0; l3Idx < 8192; ++l3Idx) {
        uint64_t virtAddr =
            (l2Idx << Granule512MiB::shift) + (l3Idx << Granule64KiB::shift);

        uint64_t physAddr;
        AttributeFields attributes;

        if (virtMemLayout()->virtAddrProperties(virtAddr, physAddr,
                                                attributes) == -1) {
          panic("Address out of range: 0x%X", virtAddr);
        }

        lvl3[l2Idx][l3Idx] =
            PageDescriptor::fromOutputAddr(physAddr, attributes);
      }
    }
  }

  uint64_t physBaseAddress() const { return reinterpret_cast<uint64_t>(lvl2); }

  template <typename T, size_t N>
  constexpr size_t phys_start_addr_usize(T (&arr)[N]) {
    return reinterpret_cast<size_t>(&arr);
  }
} __attribute__((aligned(65536)));

using KernelTranslationTable = FixedSizeTranslationTable<NUM_LVL2_TABLES>;
} // namespace Memory
