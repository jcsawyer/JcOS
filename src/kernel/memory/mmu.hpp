#pragma once

#include <common.hpp>
#include <print.hpp>
#include <stdint.h>

namespace Memory {
enum MemAttributes { CacheableDRAM, Device };

enum AccessPermissions { ReadOnly, ReadWrite };

enum MMUEnableError { AlreadyEnabled, Other };

struct AttributeFields {
  MemAttributes memAttributes;
  AccessPermissions accessPermissions;
  bool executeNever;

  static const uint64_t MEM_ATTR_OFFSET = 2;    // Offset for memAttributes
  static const uint64_t ACCESS_PERM_OFFSET = 4; // Offset for accessPermissions
  static const uint64_t EXECUTE_NEVER_OFFSET = 6; // Offset for executeNever

  static AttributeFields Default() { return {CacheableDRAM, ReadWrite, true}; }

  // User-defined conversion to uint64_t
  constexpr operator uint64_t() const {
    uint64_t result = 0;
    result |= (static_cast<uint64_t>(memAttributes) << MEM_ATTR_OFFSET);
    result |= (static_cast<uint64_t>(accessPermissions) << ACCESS_PERM_OFFSET);
    result |= (executeNever ? 1ULL : 0ULL) << EXECUTE_NEVER_OFFSET;
    return result;
  }
};

class TranslationType {
public:
  enum class Tag { Identity, Offset };

  static TranslationType Identity() {
    TranslationType t;
    t.tag_ = Tag::Identity;
    t.offset_ = 0;
    return t;
  }

  static TranslationType Offset(size_t off) {
    TranslationType t;
    t.tag_ = Tag::Offset;
    t.offset_ = off;
    return t;
  }

  bool isIdentity() const { return tag_ == Tag::Identity; }
  bool isOffset() const { return tag_ == Tag::Offset; }

  size_t offset() const { return offset_; }

private:
  Tag tag_;
  size_t offset_;
};

struct TranslationDescriptor {
  const char *name;
  size_t virtualRangeStart;
  size_t virtualRangeEnd;
  TranslationType physicalRangeTranslation;
  AttributeFields attributeFields;
};

template <size_t GRANULE_SIZE> class TranslationGranule {
public:
  static constexpr size_t size = GRANULE_SIZE;
  static constexpr size_t shift = __builtin_ctz(size);
};

template <size_t AS_SIZE> class AddressSpace {
public:
  static constexpr size_t size = AS_SIZE;
  static constexpr size_t shift = __builtin_ctz(size);
};

template <size_t NUM_SPECIAL_RANGES> class KernelVirtualLayout {
public:
  KernelVirtualLayout(size_t max,
                      TranslationDescriptor layout[NUM_SPECIAL_RANGES])
      : maxVirtualAddrInclusive(max) {
    for (size_t i = 0; i < NUM_SPECIAL_RANGES; i++) {
      inner[i] = layout[i];
    }
  }

  int virtAddrProperties(size_t virtAddr, size_t &outputAddr,
                         AttributeFields &attributes) const {
    if (virtAddr > maxVirtualAddrInclusive) {
      return -1; // Address out of range.
    }

    for (size_t i = 0; i < NUM_SPECIAL_RANGES; i++) {
      if (virtAddr >= inner[i].virtualRangeStart &&
          virtAddr <= inner[i].virtualRangeEnd) {
        outputAddr = (inner[i].physicalRangeTranslation.isIdentity())
                         ? virtAddr
                         : inner[i].physicalRangeTranslation.offset() +
                               virtAddr - inner[i].virtualRangeStart;
        attributes = inner[i].attributeFields;
        return 0; // Success.
      }
    }

    outputAddr = virtAddr;
    attributes = AttributeFields::Default();
    return 0; // Default mapping.
  }

  void printLayout() const {
    for (size_t i = 0; i < NUM_SPECIAL_RANGES; i++) {
      size_t size = inner[i].virtualRangeEnd - inner[i].virtualRangeStart + 1;
      const char *attr;
      if (inner[i].attributeFields.memAttributes ==
          MemAttributes::CacheableDRAM) {
        attr = "C";
      } else if (inner[i].attributeFields.memAttributes ==
                 MemAttributes::Device) {
        attr = "Dev";
      }
      const char *acc_p;
      if (inner[i].attributeFields.accessPermissions ==
          AccessPermissions::ReadOnly) {
        acc_p = "RO";
      } else if (inner[i].attributeFields.accessPermissions ==
                 AccessPermissions::ReadWrite) {
        acc_p = "RW";
      }

      const char *xn = inner[i].attributeFields.executeNever ? "PXN" : "PX";
      info("      0x%08X - 0x%08X | %-3s | %-3s %s %-3s | %-3s",
           inner[i].virtualRangeStart, inner[i].virtualRangeEnd,
           size_human_readable_ceil(size), attr, acc_p, xn, inner[i].name);
    }
  }

private:
  size_t maxVirtualAddrInclusive;
  TranslationDescriptor inner[NUM_SPECIAL_RANGES];
};

} // namespace Memory
