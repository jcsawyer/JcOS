#include "boot.hpp"
#include "dtb.hpp"

#include "../barrier.hpp"
#include <arch/cpu.hpp>
#include <main.hpp>
#include <memory/mmu.hpp>
#include <stdint.h>

namespace CPU {
namespace Boot {

namespace {

constexpr size_t kReleasedCoreBootStackSize = 64 * 1024;
ReleasedCoreWork gReleasedCoreWork = nullptr;

} // namespace

extern "C" {
volatile uint8_t BOOT_CORE_RELEASE_FLAGS[MAX_CORES]
    __attribute__((aligned(16), section(".data"))) = {};
uint8_t RELEASED_CORE_BOOT_STACKS[MAX_CORES][kReleasedCoreBootStackSize]
    __attribute__((aligned(16))) = {};
}

bool processDeviceTree(uintptr_t deviceTreePhysAddr) {
  return DTB::processDeviceTree(deviceTreePhysAddr);
}

bool releaseCore(size_t coreId) {
  if (coreId >= MAX_CORES) {
    return false;
  }

  __atomic_store_n(&BOOT_CORE_RELEASE_FLAGS[coreId], 1, __ATOMIC_RELEASE);
  Barrier::dmb_sy();
  CPU::sendEvent();
  return true;
}

bool isCoreReleased(size_t coreId) {
  if (coreId >= MAX_CORES) {
    return false;
  }

  return __atomic_load_n(&BOOT_CORE_RELEASE_FLAGS[coreId], __ATOMIC_ACQUIRE) !=
         0;
}

void setReleasedCoreWork(ReleasedCoreWork work) { gReleasedCoreWork = work; }

ReleasedCoreWork releasedCoreWork() { return gReleasedCoreWork; }

} // namespace Boot
} // namespace CPU

namespace {

[[noreturn]] void runReleasedCore(size_t coreId) {
  __atomic_store_n(&CPU::Boot::BOOT_CORE_RELEASE_FLAGS[coreId], 0,
                   __ATOMIC_RELEASE);

  if (CPU::Boot::ReleasedCoreWork work = CPU::Boot::releasedCoreWork();
      work != nullptr) {
    work(coreId);
  }

  CPU::waitForever();
}

void prepare_el2_to_el1_transition(
    uint64_t virt_boot_core_stack_end_exclusive_addr,
    uint64_t virt_kernel_init_addr) {
  // Enable timer counter registers for EL1
  asm volatile("msr CNTHCTL_EL2, %[value]"
               :
               : [value] "r"(0b11)); // EL1PCEN::SET + EL1PCTEN::SET

  // No offset for reading the counters
  asm volatile("msr CNTVOFF_EL2, %[value]" : : [value] "r"(0));

  // Set EL1 execution state to AArch64
  asm volatile("msr HCR_EL2, %[value]"
               :
               : [value] "r"(0b1 << 31)); // RW::EL1IsAarch64

  // Simulate exception return
  // Fake a saved program status where all interrupts are masked and SP_EL1 is
  // used as the stack pointer
  uint64_t spsr_value = 0b1 << 9 | // Mask D (Debug)
                        0b1 << 8 | // Mask A (SError)
                        0b1 << 7 | // Mask I (IRQ)
                        0b1 << 6 | // Mask F (FIQ)
                        0b0101;    // M::EL1h
  asm volatile("msr SPSR_EL2, %[value]" : : [value] "r"(spsr_value));

  // Set the link register to point to kernel_init
  asm volatile("msr ELR_EL2, %[value]" : : [value] "r"(virt_kernel_init_addr));

  // Set up SP_EL1 (stack pointer)
  asm volatile("msr SP_EL1, %[value]"
               :
               : [value] "r"(virt_boot_core_stack_end_exclusive_addr));
}

} // namespace

extern "C" void _start_cpp(uint64_t phys_kernel_tables_base_addr,
                           uint64_t virt_boot_core_stack_end_exclusive_addr,
                           uint64_t virt_kernel_init_addr,
                           uintptr_t device_tree_phys_addr) {
  uint64_t current_el = 0;
  asm volatile("mrs %0, CurrentEL" : "=r"(current_el));
  current_el &= 0b1100;

  if (current_el == 0b1000) {
    // Preserve the firmware handoff data before switching exception levels.
    CPU::Boot::processDeviceTree(device_tree_phys_addr);

    prepare_el2_to_el1_transition(virt_boot_core_stack_end_exclusive_addr,
                                  virt_kernel_init_addr);

    Memory::MMU()->enableMMUAndCaching(phys_kernel_tables_base_addr);

    // Clear the frame pointer chain before entering the higher-half kernel.
    asm volatile("mov x29, xzr");
    asm volatile("mov x30, xzr");

    // Return from the simulated EL2 exception into EL1h at kernel_init().
    asm volatile("eret");
  }

  while (true) {
    asm volatile("wfe");
  }
}

extern "C" [[noreturn]] void _released_core_main(size_t coreId) {
  runReleasedCore(coreId);
}

extern "C" [[noreturn]] void _start_released_core(
    uint64_t phys_kernel_tables_base_addr,
    uint64_t virt_released_core_stack_end_exclusive_addr,
    uint64_t virt_released_core_main_addr, size_t coreId) {
  uint64_t current_el = 0;
  asm volatile("mrs %0, CurrentEL" : "=r"(current_el));
  current_el &= 0b1100;

  if (current_el == 0b1000) {
    prepare_el2_to_el1_transition(virt_released_core_stack_end_exclusive_addr,
                                  virt_released_core_main_addr);

    Memory::MMU()->enableMMUAndCaching(phys_kernel_tables_base_addr);

    asm volatile("mov x29, xzr");
    asm volatile("mov x30, xzr");
    asm volatile("mov x0, %0" : : "r"(coreId) : "x0");
    asm volatile("eret");
  }

  if (current_el == 0b0100) {
    _released_core_main(coreId);
  }

  CPU::waitForever();
}
