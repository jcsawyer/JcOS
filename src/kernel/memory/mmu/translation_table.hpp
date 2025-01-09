#pragma once

#include "../mmu.hpp"
#include <stddef.h>
#include <stdint.h>

#include "../../_arch/aarch64/memory/mmu.hpp"

namespace Memory
{

  const size_t GRANULE_64K = 64 * 1024;
  const size_t GRANULE_512M = 512 * 1024 * 1024;

  struct TableDescriptor
  {
    uint64_t value;

    TableDescriptor() : value(0) {}

    TableDescriptor(uint64_t val) : value(val) {}

    static const uint64_t SHIFT_64KiB = 16;                            // Granule shift
    static const uint64_t NEXT_LEVEL_TABLE_ADDR_MASK = 0xFFFFFFFFF000; // [47:16]
    static const uint64_t TYPE_MASK = 1 << 1;                          // Offset 1, 1 bit
    static const uint64_t VALID_MASK = 1;                              // Offset 0, 1 bit

    static const uint64_t make_descriptor(uint64_t addr, bool valid, bool is_table)
    {
      return ((addr >> SHIFT_64KiB) & (NEXT_LEVEL_TABLE_ADDR_MASK >> SHIFT_64KiB)) |
             (is_table ? TYPE_MASK : 0) |
             (valid ? VALID_MASK : 0);
    }

    static TableDescriptor fromNextLevelTableAddr(uint64_t phys_next_lvl_table_addr)
    {
      TableDescriptor desc = TableDescriptor();
      desc.value = make_descriptor(phys_next_lvl_table_addr, true, true);
      return desc;
    }
  };

class PageDescriptor {
public:
    uint64_t value;
    
    PageDescriptor() : value(0) {}
    PageDescriptor(uint64_t val) : value(val) {}

    static constexpr uint64_t SHIFT_64KiB = 16; // Granule shift
    static constexpr uint64_t OUTPUT_ADDR_MASK = 0xFFFFFFFFF000; // [47:16]
    static constexpr uint64_t AF_MASK = 1 << 10;  // Access Flag, Offset 10
    static constexpr uint64_t TYPE_MASK = 1 << 1; // Page Type, Offset 1
    static constexpr uint64_t VALID_MASK = 1;     // Valid, Offset 0

    static constexpr uint64_t make_descriptor(uint64_t addr, const AttributeFields& attrs, bool valid, bool is_page) {
        return ((addr >> SHIFT_64KiB) & (OUTPUT_ADDR_MASK >> SHIFT_64KiB)) |
               AF_MASK |
               (is_page ? TYPE_MASK : 0) |
               (valid ? VALID_MASK : 0) |
               static_cast<uint64_t>(attrs);
    }


    static PageDescriptor fromOutputAddr(uint64_t phys_output_addr, const AttributeFields& attribute_fields) {
        PageDescriptor desc;
        desc.value = make_descriptor(phys_output_addr, attribute_fields, true, true);
        return desc;
    }
};

} // namespace Memory
