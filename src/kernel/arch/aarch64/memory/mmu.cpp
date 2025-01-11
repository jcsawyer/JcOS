#include "mmu.hpp"
#include "mmu/translation_table.hpp"

namespace Memory {

#define WRITE_SYSREG(reg, val)                                                 \
  asm volatile("msr " #reg ", %0" : : "r"(val) : "memory")
#define READ_SYSREG(reg)                                                       \
  ({                                                                           \
    uint64_t _val;                                                             \
    asm volatile("mrs %0, " #reg : "=r"(_val));                                \
    _val;                                                                      \
  })
#define DSB_SY asm volatile("dsb sy" ::: "memory")
#define ISB_SY asm volatile("isb sy" ::: "memory")

KernelTranslationTable *kernelTables();

// Configure MAIR_EL1 register
void setUpMAIR() {
  uint64_t mair_value = 0b1111 << 12 |
                        0b1111 << 8   // Attribute 1: Cacheable normal DRAM
                        | 0b00000100; // Attribute 0: Device

  WRITE_SYSREG(mair_el1, mair_value);
}

// Configure the translation control register
void configureTranslationControl() {
  uint64_t t0sz = 64 - KernelAddrSpace::shift;

  uint64_t tcr_value =
      0b0ULL << 37     // TBIO Used
      | 0b010ULL << 32 // IPS 40-bit physical address size
      | 0b01ULL << 14  // TG0 64 KiB granule
      | 0b11ULL << 12  // SH0 Inner Shareable
      | 0b01ULL << 10  // ORGN0 Write-back, Read-allocate, Cacheable
      | 0b01ULL << 8   // IRGN0 Write-back, Read-allocate, Cacheable
      | 0b0ULL << 7    // EPD0 Enable TTBR0 walks
      | 0b0ULL << 22   // A1 Use TTBR0
      | 0b1ULL << 23   // EPD1 Disable TTBR1Walks
      | t0sz << 0;     // T0SZ Address size for TTBR0

  WRITE_SYSREG(tcr_el1, tcr_value);
}

// Check if the MMU is enabled
bool MemoryManagementUnit::isEnabled() {
  uint64_t sctlr_value = READ_SYSREG(sctlr_el1);
  return sctlr_value & 0b1 << 0; // Check if the M (MMU enable) bit is set.
}

// Enable MMU and caching
void MemoryManagementUnit::enableMMUAndCaching() {
  // Check if MMU is already enabled.
  if (isEnabled()) {
    panic("MMU is already enabled");
  }

  // Read the translation granule support field from ID_AA64MMFR0_EL1.
  uint64_t mmfr0 = READ_SYSREG(id_aa64mmfr0_el1);
  // Check if 64 KiB granule is unsupported.
  if (uint64_t granule_support = mmfr0 >> 28 & 0xF; granule_support != 0x0) {
    panic("64 KiB granule is not supported");
  }

  // Set up MAIR_EL1.
  setUpMAIR();

  // Populate translation tables.
  // Assuming populateTTEntries() initializes the translation tables.
  kernelTables()->populateTTEntries();

  // Set the Translation Table Base Register (TTBR0_EL1).
  uint64_t ttbr0_value = kernelTables()->physBaseAddress();
  WRITE_SYSREG(ttbr0_el1, ttbr0_value);

  // Configure translation control settings.
  configureTranslationControl();

  // Ensure all memory operations are complete before enabling the MMU.
  // DSB_SY;
  ISB_SY;

  // Enable the MMU and caching in SCTLR_EL1.
  uint64_t sctlr_value = READ_SYSREG(sctlr_el1);
  sctlr_value |= 1 << 0;  // M: Enable MMU
  sctlr_value |= 1 << 2;  // C: Enable data cache
  sctlr_value |= 1 << 12; // I: Enable instruction cache
  WRITE_SYSREG(sctlr_el1, sctlr_value);

  // Ensure the changes take effect before continuing.
  ISB_SY;
}

KernelTranslationTable *kernelTables() {
  static auto kernelTables = KernelTranslationTable();
  return &kernelTables;
}

MemoryManagementUnit *MMU() {
  static auto mmu = MemoryManagementUnit();
  return &mmu;
}

} // namespace Memory
