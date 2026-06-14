#include "mmu.hpp"
#include "../../../memory/mmu.hpp"
#include "../memory.hpp"
#include <arch/aarch64/memory/mmu/translation_table.hpp>
#include <container/vector.hpp>
#include <panic.hpp>
#include <task.hpp>

namespace Memory {
extern "C" {
KernelTranslationTable KERNEL_TABLES
    __attribute__((section(".data"), aligned(65536))) = {};

uint64_t PHYS_KERNEL_TABLES_BASE_ADDR
    __attribute__((section(".text._start_arguments"), used)) =
        0xCCCCAAAAFFFFEEEEULL;
}

namespace {
const size_t KERNEL_GRANULE_SIZE = 64 * 1024;
const size_t TABLE_WIDTH = 139;

struct MMIORecord {
  size_t physPageStart;
  size_t virtPageStart;
  size_t numPages;
  Container::Vector<const char *> entities;
};

Container::Vector<MMIORecord> mmioRecords;

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

void formatVirtualAddress(char *out, size_t outLen, size_t addr) {
  uint64_t value = static_cast<uint64_t>(addr);
  snprintf_(out, outLen, "0x%04lx_%04lx_%04lx_%04lx",
            static_cast<size_t>((value >> 48) & 0xFFFF),
            static_cast<size_t>((value >> 32) & 0xFFFF),
            static_cast<size_t>((value >> 16) & 0xFFFF),
            static_cast<size_t>(value & 0xFFFF));
}

void formatPhysicalAddress(char *out, size_t outLen, size_t addr) {
  uint64_t value = static_cast<uint64_t>(addr);
  snprintf_(out, outLen, "0x%02lx_%04lx_%04lx",
            static_cast<size_t>((value >> 32) & 0xFF),
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

  formatVirtualAddress(virtStartStr, sizeof(virtStartStr), virtStart);
  formatVirtualAddress(virtEndStr, sizeof(virtEndStr), virtEndInclusive);
  formatPhysicalAddress(physStartStr, sizeof(physStartStr), physStart);
  formatPhysicalAddress(physEndStr, sizeof(physEndStr), physEndInclusive);
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
          TranslationType::Offset(physCodeStart()),
          {CacheableDRAM, ReadOnly, false},
      },
      {
          "Kernel data and bss",
          dataStart(),
          dataEndExclusive() - 1,
          TranslationType::Offset(physDataStart()),
          {CacheableDRAM, ReadWrite, true},
      },
      {
          "Kernel heap",
          heapStart(),
          heapEndExclusive() - 1,
          TranslationType::Offset(physHeapStart()),
          {CacheableDRAM, ReadWrite, true},
      },
      {
          "Remapped Device MMIO",
          mmioRemapStart(),
          mmioRemapEndExclusive() - 1,
          TranslationType::Offset(Map::getMMIO().START),
          {Device, ReadWrite, true},
      },
      {
          "Kernel boot-core stack",
          bootCoreStackStart(),
          bootCoreStackEndExclusive() - 1,
          TranslationType::Offset(physBootCoreStackStart()),
          {CacheableDRAM, ReadWrite, true},
      },
  };

  return descriptors;
}

static KernelVirtualLayout<NUM_MEM_RANGES> *kernelVirtualLayout = nullptr;

KernelVirtualLayout<NUM_MEM_RANGES> *virtMemLayout() {
  if (kernelVirtualLayout == nullptr) {
    static auto kvl = KernelVirtualLayout<NUM_MEM_RANGES>(
        Map::KERNEL_VIRT_END_INCLUSIVE, getDescriptors());
    kernelVirtualLayout = &kvl;
  }

  return kernelVirtualLayout;
}

size_t kernelVirtToPhys(size_t virtAddr) {
  size_t physAddr;
  AttributeFields attributes;
  if (virtMemLayout()->virtAddrProperties(virtAddr, physAddr, attributes) !=
      0) {
    panic("Virtual address out of range: 0x%016lX", virtAddr);
  }

  return physAddr;
}

size_t kernelMapMMIO(const char *name, const MMIODescriptor &descriptor) {
  if (descriptor.size == 0) {
    panic("MMIO map '%s' requested with zero size", name);
  }

  const size_t physPageStart = alignDownToGranule(descriptor.startAddr);
  const size_t offsetIntoPage = descriptor.startAddr - physPageStart;
  const size_t numPages =
      div_ceil(offsetIntoPage + descriptor.size, KERNEL_GRANULE_SIZE);

  for (size_t i = 0; i < mmioRecords.size(); i++) {
    if (mmioRecords[i].physPageStart == physPageStart &&
        mmioRecords[i].numPages == numPages) {
      bool duplicate = false;
      for (size_t entityIdx = 0; entityIdx < mmioRecords[i].entities.size();
           entityIdx++) {
        if (mmioRecords[i].entities[entityIdx] == name) {
          duplicate = true;
          break;
        }
      }
      if (!duplicate) {
        mmioRecords[i].entities.pushBack(name);
      }
      return mmioRecords[i].virtPageStart + offsetIntoPage;
    }
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
      mmioRemapStart() + (physPageStart - physMmioStart);
  const size_t virtEndInclusive =
      virtPageStart + numPages * KERNEL_GRANULE_SIZE - 1;

  if (virtEndInclusive >= mmioRemapEndExclusive()) {
    panic("MMIO remap region exhausted");
  }

  MMIORecord record{};
  record.physPageStart = physPageStart;
  record.virtPageStart = virtPageStart;
  record.numPages = numPages;
  record.entities.pushBack(name);
  mmioRecords.pushBack(static_cast<MMIORecord &&>(record));

  return virtPageStart + offsetIntoPage;
}

void kernelPrintMappings() {
  const size_t dataVirtEndExclusive =
      dataStart() +
      div_ceil(dataEndExclusive() - dataStart(), KERNEL_GRANULE_SIZE) *
          KERNEL_GRANULE_SIZE;
  const size_t heapVirtEndExclusive =
      heapStart() +
      div_ceil(heapEndExclusive() - heapStart(), KERNEL_GRANULE_SIZE) *
          KERNEL_GRANULE_SIZE;
  const size_t bootStackVirtEndExclusive =
      bootCoreStackStart() +
      div_ceil(bootCoreStackEndExclusive() - bootCoreStackStart(),
               KERNEL_GRANULE_SIZE) *
          KERNEL_GRANULE_SIZE;

  printDivider();
  info("                        Virtual                                   "
       "Physical               Size       Attr                    Entity");
  printDivider();
  printMappedRow(codeStart(), codeEndExclusive() - 1, physCodeStart(),
                 physCodeStart() + (codeEndExclusive() - codeStart()) - 1,
                 codeEndExclusive() - codeStart(), "C   RO X",
                 "Kernel code and RO data");
  printMappedRow(dataStart(), dataVirtEndExclusive - 1, physDataStart(),
                 physDataStart() + (dataVirtEndExclusive - dataStart()) - 1,
                 dataVirtEndExclusive - dataStart(), "C   RW XN",
                 "Kernel data and bss");
  printMappedRow(heapStart(), heapVirtEndExclusive - 1, physHeapStart(),
                 physHeapStart() + (heapVirtEndExclusive - heapStart()) - 1,
                 heapVirtEndExclusive - heapStart(), "C   RW XN",
                 "Kernel heap");
  printMappedRow(bootCoreStackStart(), bootStackVirtEndExclusive - 1,
                 physBootCoreStackStart(),
                 physBootCoreStackStart() +
                     (bootStackVirtEndExclusive - bootCoreStackStart()) - 1,
                 bootStackVirtEndExclusive - bootCoreStackStart(), "C   RW XN",
                 "Kernel boot-core stack");

  for (size_t i = 0; i < mmioRecords.size(); i++) {
    const size_t virtStart = mmioRecords[i].virtPageStart;
    const size_t virtEndInclusive =
        virtStart + mmioRecords[i].numPages * KERNEL_GRANULE_SIZE - 1;
    const size_t physStart = mmioRecords[i].physPageStart;
    const size_t physEndInclusive =
        physStart + mmioRecords[i].numPages * KERNEL_GRANULE_SIZE - 1;
    printMappedRow(virtStart, virtEndInclusive, physStart, physEndInclusive,
                   mmioRecords[i].numPages * KERNEL_GRANULE_SIZE, "Dev RW XN",
                   mmioRecords[i].entities[0]);
    for (size_t aliasIdx = 1; aliasIdx < mmioRecords[i].entities.size();
         aliasIdx++) {
      printEntityContinuation(mmioRecords[i].entities[aliasIdx]);
    }
  }
  printDivider();
}

bool isValidCodeAddress(size_t address) {
  return address >= codeStart() && address < codeEndExclusive();
}

bool isValidCurrentStackAddress(size_t address) {
  if (address >= bootCoreStackStart() &&
      address < bootCoreStackEndExclusive()) {
    return true;
  }

  Task *currentTask = taskManager.current();
  if (currentTask == nullptr) {
    return false;
  }

  return address >= currentTask->stackStart() &&
         address < currentTask->stackEndExclusive();
}
} // namespace Memory
