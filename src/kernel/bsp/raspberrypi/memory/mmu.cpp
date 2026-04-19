#include "mmu.hpp"
#include "../../../memory/mmu.hpp"
#include "../memory.hpp"
#include <panic.hpp>

namespace Memory {
namespace {
const size_t KERNEL_GRANULE_SIZE = 64 * 1024;
const size_t MAX_MMIO_MAPPINGS = 32;
const size_t MAX_MMIO_ALIASES = 8;
const size_t TABLE_WIDTH = 139;

struct MMIORecord {
  size_t physPageStart;
  size_t virtPageStart;
  size_t numPages;
  const char *entities[MAX_MMIO_ALIASES];
  size_t entityCount;
};

MMIORecord mmioRecords[MAX_MMIO_MAPPINGS] = {};
size_t mmioRecordCount = 0;

size_t alignDownToGranule(size_t value) {
  return value & ~(KERNEL_GRANULE_SIZE - 1);
}

void printDivider() {
  static char divider[TABLE_WIDTH + 1] = {};
  if (divider[0] == '\0') {
    for (size_t i = 0; i < TABLE_WIDTH; i++) {
      divider[i] = '-';
    }
    divider[TABLE_WIDTH] = '\0';
  }
  info("      %s", divider);
}

void formatAddress(char *out, size_t outLen, size_t addr) {
  uint64_t value = static_cast<uint64_t>(addr);
  snprintf_(out, outLen, "0x%04lX_%04lX_%04lX_%04lX",
            static_cast<size_t>((value >> 48) & 0xFFFF),
            static_cast<size_t>((value >> 32) & 0xFFFF),
            static_cast<size_t>((value >> 16) & 0xFFFF),
            static_cast<size_t>(value & 0xFFFF));
}

void formatSize(char *out, size_t outLen, size_t bytes) {
  if ((bytes % MiB) == 0) {
    snprintf_(out, outLen, "%4lu MiB", bytes / MiB);
    return;
  }
  if ((bytes % KiB) == 0) {
    snprintf_(out, outLen, "%4lu KiB", bytes / KiB);
    return;
  }
  snprintf_(out, outLen, "%4lu B", bytes);
}

void printMappedRow(size_t virtStart, size_t virtEndInclusive, size_t physStart,
                    size_t physEndInclusive, size_t bytes, const char *attr,
                    const char *entity) {
  char virtStartStr[24];
  char virtEndStr[24];
  char physStartStr[24];
  char physEndStr[24];
  char sizeStr[16];

  formatAddress(virtStartStr, sizeof(virtStartStr), virtStart);
  formatAddress(virtEndStr, sizeof(virtEndStr), virtEndInclusive);
  formatAddress(physStartStr, sizeof(physStartStr), physStart);
  formatAddress(physEndStr, sizeof(physEndStr), physEndInclusive);
  formatSize(sizeStr, sizeof(sizeStr), bytes);

  info("      %s..%s --> %s..%s | %7s | %-10s | %s", virtStartStr, virtEndStr,
       physStartStr, physEndStr, sizeStr, attr, entity);
}

void printEntityContinuation(const char *entity) {
  static char entityColumnPrefix[256] = {};
  if (entityColumnPrefix[0] == '\0') {
    char sampleRow[256] = {};
    constexpr const char *dummyAddr = "0x0000_0000_0000_0000";
    snprintf_(sampleRow, sizeof(sampleRow),
              "      %s..%s --> %s..%s | %7s | %-10s | %s", dummyAddr,
              dummyAddr, dummyAddr, dummyAddr, "64 KiB", "Dev RW XN", "DUMMY");

    size_t lastBar = 0;
    for (size_t i = 0; sampleRow[i] != '\0'; i++) {
      if (sampleRow[i] == '|') {
        lastBar = i;
      }
    }

    for (size_t i = 0; i < lastBar; i++) {
      entityColumnPrefix[i] = ' ';
    }
    entityColumnPrefix[lastBar] = '|';
    entityColumnPrefix[lastBar + 1] = ' ';
    entityColumnPrefix[lastBar + 2] = '\0';
  }

  info("%s%s", entityColumnPrefix, entity);
}
} // namespace

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
          Map::MMIO_REMAP_START,
          Map::MMIO_REMAP_END_INCLUSIVE,
          TranslationType::Offset(Map::getMMIO().START),
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

size_t kernelMapMMIO(const char *name, const MMIODescriptor &descriptor) {
  if (descriptor.size == 0) {
    panic("MMIO map '%s' requested with zero size", name);
  }

  const size_t physPageStart = alignDownToGranule(descriptor.startAddr);
  const size_t offsetIntoPage = descriptor.startAddr - physPageStart;
  const size_t numPages =
      div_ceil(offsetIntoPage + descriptor.size, KERNEL_GRANULE_SIZE);

  for (size_t i = 0; i < mmioRecordCount; i++) {
    if (mmioRecords[i].physPageStart == physPageStart &&
        mmioRecords[i].numPages == numPages) {
      if (mmioRecords[i].entityCount < MAX_MMIO_ALIASES) {
        bool duplicate = false;
        for (size_t entityIdx = 0; entityIdx < mmioRecords[i].entityCount;
             entityIdx++) {
          if (mmioRecords[i].entities[entityIdx] == name) {
            duplicate = true;
            break;
          }
        }
        if (!duplicate) {
          mmioRecords[i].entities[mmioRecords[i].entityCount++] = name;
        }
      }
      return mmioRecords[i].virtPageStart + offsetIntoPage;
    }
  }

  if (mmioRecordCount >= MAX_MMIO_MAPPINGS) {
    panic("Exceeded max MMIO mappings");
  }

  const size_t physMmioStart = Map::getMMIO().START;
  const size_t physMmioEndExclusive = Map::getMMIO().END_INCLUSIVE + 1;
  const size_t physEndExclusive =
      physPageStart + numPages * KERNEL_GRANULE_SIZE;

  if (physPageStart < physMmioStart ||
      physEndExclusive > physMmioEndExclusive) {
    panic("MMIO map outside physical MMIO range");
  }

  const size_t virtPageStart =
      Map::MMIO_REMAP_START + (physPageStart - physMmioStart);
  const size_t virtEndInclusive =
      virtPageStart + numPages * KERNEL_GRANULE_SIZE - 1;

  if (virtEndInclusive > Map::MMIO_REMAP_END_INCLUSIVE) {
    panic("MMIO remap region exhausted");
  }

  MMIORecord &record = mmioRecords[mmioRecordCount++];
  record.physPageStart = physPageStart;
  record.virtPageStart = virtPageStart;
  record.numPages = numPages;
  record.entityCount = 1;
  record.entities[0] = name;
  for (size_t i = 1; i < MAX_MMIO_ALIASES; i++) {
    record.entities[i] = nullptr;
  }

  return virtPageStart + offsetIntoPage;
}

void kernelPrintMappings() {
  const size_t dataVirtEndExclusive =
      div_ceil(dataEndExclusive(), KERNEL_GRANULE_SIZE) * KERNEL_GRANULE_SIZE;

  printDivider();
  info("                        Virtual                                   "
       "Physical               Size       Attr                    Entity");
  printDivider();
  printMappedRow(bootCoreStackStart(), bootCoreStackEndExclusive() - 1,
                 bootCoreStackStart(), bootCoreStackEndExclusive() - 1,
                 bootCoreStackEndExclusive() - bootCoreStackStart(),
                 "C   RW XN", "Kernel boot-core stack");
  printMappedRow(codeStart(), codeEndExclusive() - 1, codeStart(),
                 codeEndExclusive() - 1, codeEndExclusive() - codeStart(),
                 "C   RO X", "Kernel code and RO data");
  printMappedRow(dataStart(), dataVirtEndExclusive - 1, dataStart(),
                 dataVirtEndExclusive - 1, dataVirtEndExclusive - dataStart(),
                 "C   RW XN", "Kernel data and bss");

  for (size_t i = 0; i < mmioRecordCount; i++) {
    const size_t virtStart = mmioRecords[i].virtPageStart;
    const size_t virtEndInclusive =
        virtStart + mmioRecords[i].numPages * KERNEL_GRANULE_SIZE - 1;
    const size_t physStart = mmioRecords[i].physPageStart;
    const size_t physEndInclusive =
        physStart + mmioRecords[i].numPages * KERNEL_GRANULE_SIZE - 1;
    printMappedRow(virtStart, virtEndInclusive, physStart, physEndInclusive,
                   mmioRecords[i].numPages * KERNEL_GRANULE_SIZE, "Dev RW XN",
                   mmioRecords[i].entities[0]);
    for (size_t aliasIdx = 1; aliasIdx < mmioRecords[i].entityCount;
         aliasIdx++) {
      printEntityContinuation(mmioRecords[i].entities[aliasIdx]);
    }
  }
  printDivider();
}
} // namespace Memory
