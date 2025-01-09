#include "mmu.hpp"
#include "../barrier.hpp"
#include "barrier.hpp"
#include "mmu/translation_table.hpp"

namespace Memory
{

#define WRITE_SYSREG(reg, val) asm volatile("msr " #reg ", %0" : : "r"(val) : "memory")
#define READ_SYSREG(reg) ({ uint64_t _val; asm volatile("mrs %0, " #reg : "=r"(_val)); _val; })
#define DSB_SY asm volatile("dsb sy" ::: "memory")
#define ISB_SY asm volatile("isb sy" ::: "memory")

  // Define MAIR_EL1 values.
  const uint64_t MAIR_ATTR_DEVICE = 0x04; // Device: nonGathering, nonReordering, EarlyWriteAck
  const uint64_t MAIR_ATTR_NORMAL = 0xFF; // Normal: WriteBack NonTransient ReadWriteAlloc

  // Configure MAIR_EL1 register
  void MemoryManagementUnit::setUpMAIR()
  {
    uint64_t mair_value =
        (MAIR_ATTR_NORMAL << 8) | // Attribute 1: Cacheable normal DRAM
        (MAIR_ATTR_DEVICE << 0);  // Attribute 0: Device

    WRITE_SYSREG(mair_el1, mair_value);
  }

  // Configure the translation control register
  void MemoryManagementUnit::configureTranslationControl()
  {
    uint64_t t0sz = static_cast<uint64_t>(64 - KernelAddrSpace::shift);

    // uint64_t tcr_value =
    //     (1ULL << 37) | // TBI0: Used
    //     (2ULL << 32) | // IPS: 40-bit physical address size
    //     (0b10ULL << 14) | // TG0: 64 KiB granule
    //     (3ULL << 12) | // SH0: Inner shareable
    //     (1ULL << 10) | // ORGN0: Write-back, ReadAlloc/WriteAlloc
    //     (1ULL << 8) |  // IRGN0: Write-back, ReadAlloc/WriteAlloc
    //     (0ULL << 7) |  // EPD0: Enable TTBR0 walks
    //     (0ULL << 6) |  // A1: Use TTBR0
    //     (t0sz << 0);   // T0SZ: Address size for TTBR0

    //info("tsz: %d", t0sz);

    uint64_t tcr_value = (0b00LL << 37)   // TBI0
                         | (0xF << 32)    // IPS (40 bits)
                         | (0b00LL << 14) // TG0 (64 KiB)
                         | (0b11LL << 12) // SH0 (Inner Shareable)
                         | (1 << 10)      // ORGN0 (WriteBack)
                         | (1 << 8)       // IRGN0 (WriteBack)
                         | (1 << 7)       // EPD0 (Enable TTBR0 walks)
                         | (0 << 6)       // A1 (Use TTBR0)
                         | (t0sz << 0);   // T0SZ
    WRITE_SYSREG(tcr_el1, tcr_value);
  }

  // Check if the MMU is enabled
  bool MemoryManagementUnit::isEnabled() const
  {
    uint64_t sctlr_value = READ_SYSREG(sctlr_el1);
    return sctlr_value & (1 << 0); // Check if the M (MMU enable) bit is set.
  }

  // Enable MMU and caching
  void MemoryManagementUnit::enableMMUAndCaching()
  {
    // Check if MMU is already enabled.
    if (isEnabled())
    {
      panic("MMU is already enabled");
    }

    // Read the translation granule support field from ID_AA64MMFR0_EL1.
    uint64_t mmfr0 = READ_SYSREG(id_aa64mmfr0_el1);
    uint64_t granule_support = (mmfr0 >> 28) & 0xF;
    if (granule_support != 0x0)
    { // Check if 64 KiB granule is unsupported.
      panic("64 KiB granule is not supported");
    }

    // Set up MAIR_EL1.
    setUpMAIR();

    // Populate translation tables.
    // Assuming populateTTEntries() initializes the translation tables.
    kernelTables()->populateTTEntries();

    // Set the Translation Table Base Register (TTBR0_EL1).
    //info("Setting TTBR0_EL1 to 0x%x", kernelTables()->physBaseAddress());
    WRITE_SYSREG(ttbr0_el1, kernelTables()->physBaseAddress());

    // Configure translation control settings.
    configureTranslationControl();

    // Ensure all memory operations are complete before enabling the MMU.
    DSB_SY;
    ISB_SY;

    // Enable the MMU and caching in SCTLR_EL1.
    uint64_t sctlr_value = READ_SYSREG(sctlr_el1);
    sctlr_value |= (1 << 0);  // M: Enable MMU
    sctlr_value |= (1 << 2);  // C: Enable data cache
    sctlr_value |= (1 << 12); // I: Enable instruction cache
    WRITE_SYSREG(sctlr_el1, sctlr_value);

    // Ensure the changes take effect before continuing.
    ISB_SY;
  }

  KernelTranslationTable *kernelTables()
  {
    static KernelTranslationTable kernelTables = KernelTranslationTable();
    return &kernelTables;
  }

  MemoryManagementUnit *MMU()
  {
    static MemoryManagementUnit mmu = MemoryManagementUnit();
    return &mmu;
  }

} // namespace Memory
