#pragma once

#include <stdint.h>

#include "../../_arch/aarch64/memory/mmu/translation_table.hpp"

namespace Memory {

const size_t GRANULE_64K = 64 * 1024;
const size_t GRANULE_512M = 512 * 1024 * 1024;

struct TableDescriptor {
  uint64_t value;

  TableDescriptor() : value(0) {}

  TableDescriptor(uint64_t val) : value(val) {}

  static const uint64_t SHIFT_64KiB = 16; // Granule shift

  static TableDescriptor
  fromNextLevelTableAddr(uint64_t phys_next_lvl_table_addr) {
    TableDescriptor desc = TableDescriptor();
    uint64_t shifted = phys_next_lvl_table_addr >> SHIFT_64KiB;
    desc.value = (shifted & 0xFFFFFFFFFFF) << SHIFT_64KiB |
                 0b11; // Set valid and table bits
    return desc;
  }
};

class PageDescriptor {
public:
  uint64_t value;

  PageDescriptor() : value(0) {}
  PageDescriptor(uint64_t val) : value(val) {}

  static constexpr uint64_t SHIFT_64KiB = 16; // Granule shift

  static uint64_t fromAttributeFields(const AttributeFields &attribute_fields) {
    uint64_t desc = 0;

    // Memory attributes
    if (attribute_fields.memAttributes == MemAttributes::CacheableDRAM) {
      desc |= (0b11ULL << 8) | (0b001ULL << 2);
    } else if (attribute_fields.memAttributes == MemAttributes::Device) {
      desc |= (0b010ULL << 8) | (0b000ULL << 2);
    }

    // Access permissions
    if (attribute_fields.accessPermissions == AccessPermissions::ReadOnly) {
      desc |= 0b10ULL << 6;
    } else if (attribute_fields.accessPermissions ==
               AccessPermissions::ReadWrite) {
      desc |= 0b00ULL << 6;
    }

    // Execute-never attribute
    desc |= (attribute_fields.executeNever ? 0b1ULL : 0b0ULL) << 53;

    // Always set unprivileged execute-never
    desc |= (0b1ULL << 54);

    return desc;
  }

  static PageDescriptor
  fromOutputAddr(uint64_t phys_output_addr,
                 const AttributeFields &attribute_fields) {
    PageDescriptor desc;
    uint64_t shifted = phys_output_addr >> SHIFT_64KiB;
    desc.value = (shifted & 0xFFFFFFFFFFF) << SHIFT_64KiB | (1 << 10) |
                 0b11 // Set valid and table bits
                 | fromAttributeFields(attribute_fields);

    return desc;
  }
};

} // namespace Memory
