#pragma once

#include "../mmu.hpp"
#include "../../../memory/mmu/translation_table.hpp"
#include "../../../bsp/raspberrypi/memory/mmu.hpp"
#include <stddef.h>
#include <stdint.h>
#include <panic.hpp>

namespace Memory
{

  using Granule512MiB = TranslationGranule<GRANULE_512M>;
  using Granule64KiB = TranslationGranule<GRANULE_64K>;

  const size_t NUM_LVL2_TABLES = KernelAddrSpace::size >> Granule512MiB::shift;

  template <size_t NUM_TABLES>
  struct FixedSizeTranslationTable
  {
    PageDescriptor lvl3[NUM_TABLES][8192];
    TableDescriptor lvl2[NUM_TABLES];

    FixedSizeTranslationTable() : lvl3(), lvl2() {}

    void populateTTEntries()
    {
      //info("lv2: %d, lv3\n", lvl2[0].value);
      for (size_t l2Idx = 0; l2Idx < NUM_TABLES; ++l2Idx)
      {
        lvl2[l2Idx] =
            TableDescriptor::fromNextLevelTableAddr((size_t)&lvl3[l2Idx]);

        for (size_t l3Idx = 0; l3Idx < 8192; ++l3Idx)
        {
          size_t virtAddr = (l2Idx * GRANULE_512M) + (l3Idx * GRANULE_64K);
          size_t physAddr;
          AttributeFields attributes;

          if (virtMemLayout()->virtAddrProperties(virtAddr, physAddr, attributes) == -1) {
            panic("Address out of range: 0x%X", virtAddr);
          }

          lvl3[l2Idx][l3Idx] =
              PageDescriptor::fromOutputAddr(physAddr, attributes);

          if (physAddr >= Memory::Map::getMMIO().START && physAddr <= Memory::Map::getMMIO().END_INCLUSIVE)
          {
            info("lvl2: 0x%X, lvl3: 0x%X, virtAddress: 0x%X, physAddress: 0x%X", lvl2[l2Idx].value, lvl3[l2Idx][l3Idx].value, virtAddr, physAddr);
          }

          //info("lvl2: 0x%X, lvl3: 0x%X, virtAddress: 0x%X, physAddress: 0x%X", lvl2[l2Idx].value, lvl3[l2Idx][l3Idx].value, virtAddr, physAddr);
        }
      }
    }

    uint64_t physBaseAddress() const { return reinterpret_cast<uint64_t>(lvl2); }

    template <typename T, size_t N>
    static inline size_t phys_start_addr_usize(T (&arr)[N])
    {
      return reinterpret_cast<size_t>(&arr);
    }
  } __attribute__((aligned(65536)));

  using KernelTranslationTable = FixedSizeTranslationTable<NUM_LVL2_TABLES>;
} // namespace Memory
