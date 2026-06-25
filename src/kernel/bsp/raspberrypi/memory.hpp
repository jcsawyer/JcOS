#pragma once
#include <stddef.h>

#include <print.hpp>

//! BSP Memory Management.
//!
//! The kernel's higher-half virtual memory layout.
//!
//! The Raspberry's firmware copies the kernel binary to physical address
//! 0x8_0000, but the kernel executes from the top 1 GiB of the 64-bit virtual
//! address space.
//!
//! +---------------------------------------+
//! |                                       | code_start @ 0xffff_ffff_c000_0000
//! | .text                                 |
//! | .rodata                               |
//! | .kernel_symbols                       |
//! |                                       |
//! +---------------------------------------+
//! |                                       | code_end_exclusive
//! | .data                                 |
//! | .bss                                  |
//! |                                       |
//! +---------------------------------------+
//! |                                       | heap_start
//! | Kernel Heap                           |
//! |                                       |
//! +---------------------------------------+
//! | MMIO remap reserved                   |
//! |                                       |
//! +---------------------------------------+
//! | Unmapped guard page                   |
//! +---------------------------------------+
//! |                                       | boot_core_stack_start
//! | Boot-core Stack                       |                                ^
//! |                                       |                                |
//! stack |                                       | | growth | | | direction
//! +---------------------------------------+
//! |                                       | boot_core_stack_end_exclusive
//! |                                       |
//! |                                       |

namespace Memory {
struct MMIO {
  size_t START;
  size_t VIDEOCORE_MBOX_START;
  size_t RNG_START;
  size_t GPIO_START;
  size_t PL011_UART_START;
  size_t TIMER_START;
  size_t LOCAL_INTERRUPT_CONTROLLER_START;
  size_t END_INCLUSIVE;

  MMIO(size_t start, size_t videocore_mbox_start, size_t rng_start,
       size_t gpio_start, size_t uart_start, size_t timer_start,
       size_t local_interrupt_controller_start, size_t end_inclusive)
      : START(start), VIDEOCORE_MBOX_START(videocore_mbox_start),
        RNG_START(rng_start), GPIO_START(gpio_start),
        PL011_UART_START(uart_start), TIMER_START(timer_start),
        LOCAL_INTERRUPT_CONTROLLER_START(local_interrupt_controller_start),
        END_INCLUSIVE(end_inclusive) {}
};
struct Map {
  static const size_t BOARD_DEFAULT_LOAD_ADDRESS = 0x80000;
  static const size_t KERNEL_VIRT_ADDR_SPACE_SIZE = 1ULL << 30;
  static const size_t KERNEL_VIRT_START =
      (~(KERNEL_VIRT_ADDR_SPACE_SIZE - 1ULL));
  static const size_t KERNEL_VIRT_END_INCLUSIVE = ~0ULL;

  static const size_t VIDEOCORE_MBOX_OFFSET = 0x0000B880;
  static const size_t VIDEOCORE_MBOX_SIZE = 0x24;
  static const size_t RNG_OFFSET = 0x00104000;
  static const size_t RNG_SIZE = 0x14;
  static const size_t GPIO_OFFSET = 0x00200000;
  static const size_t GPIO_SIZE = 0xE8;
  static const size_t SPI0_OFFSET = 0x00204000;
  static const size_t SPI0_SIZE = 0x18;
  static const size_t UART_OFFSET = 0x00201000;
  static const size_t UART_SIZE = 0x48;
  static const size_t TIMER_OFFSET = 0x00003000;
  static const size_t TIMER_SIZE = 0x14;
  static const size_t LOCAL_INTERRUPT_CONTROLLER_START = 0x40000000;
  static const size_t LOCAL_INTERRUPT_CONTROLLER_SIZE = 0x100;
  static const size_t INTERRUPT_CONTROLLER_OFFSET = 0x0000B200;
  static const size_t INTERRUPT_CONTROLLER_SIZE = 0x24;
  static const size_t MMIO_REMAP_SIZE = 0x02000000;

  inline static MMIO getMMIO() {
#if BOARD == bsp_rpi3
    return MMIO(0x3F000000, 0x3F000000 + VIDEOCORE_MBOX_OFFSET,
                0x3F000000 + RNG_OFFSET, 0x3F000000 + GPIO_OFFSET,
                0x3F000000 + UART_OFFSET, 0x3F000000 + TIMER_OFFSET,
                LOCAL_INTERRUPT_CONTROLLER_START, 0x4000FFFF);
#elif BOARD == bsp_rpi4
    return MMIO(0xFE000000, 0xFE000000 + VIDEOCORE_MBOX_OFFSET,
                0xFE000000 + RNG_OFFSET, 0xFE000000 + GPIO_OFFSET,
                0xFE000000 + UART_OFFSET, 0xFE000000 + TIMER_OFFSET, 0,
                0xFF84FFFF);
#else
#error Unknown board
#endif
  }
};

extern "C" {
extern size_t __kernel_virt_addr_space_size;
extern size_t __kernel_virt_start_addr;
extern size_t __code_start;
extern size_t __code_end_exclusive;
extern size_t __data_start;
extern size_t __data_end_exclusive;
extern size_t __heap_start;
extern size_t __heap_end_exclusive;
extern size_t __mmio_remap_start;
extern size_t __mmio_remap_end_exclusive;
extern size_t __boot_core_stack_start;
extern size_t __bss_end_exclusive;
extern size_t __boot_core_stack_end_exclusive;
extern size_t __phys_code_start;
extern size_t __phys_data_start;
extern size_t __phys_heap_start;
extern size_t __phys_boot_core_stack_start;

inline static size_t kernelVirtStart() {
  return reinterpret_cast<size_t>(&__kernel_virt_start_addr);
}

inline static size_t codeStart() {
  // The start address of the code segment is provided by the linker script.
  return reinterpret_cast<size_t>(&__code_start);
}

inline static size_t codeEndExclusive() {
  // The end address of the code segment (exclusive) is provided by the
  // linker script.
  return reinterpret_cast<size_t>(&__code_end_exclusive);
}

inline static size_t dataStart() {
  return reinterpret_cast<size_t>(&__data_start);
}

inline static size_t dataEndExclusive() {
  return reinterpret_cast<size_t>(&__data_end_exclusive);
}

inline static size_t heapStart() {
  return reinterpret_cast<size_t>(&__heap_start);
}

inline static size_t heapEndExclusive() {
  return reinterpret_cast<size_t>(&__heap_end_exclusive);
}

inline static size_t mmioRemapStart() {
  return reinterpret_cast<size_t>(&__mmio_remap_start);
}

inline static size_t mmioRemapEndExclusive() {
  return reinterpret_cast<size_t>(&__mmio_remap_end_exclusive);
}

inline static size_t bootCoreStackStart() {
  return reinterpret_cast<size_t>(&__boot_core_stack_start);
}

inline static size_t bootCoreStackEndExclusive() {
  return reinterpret_cast<size_t>(&__boot_core_stack_end_exclusive);
}

inline static size_t physCodeStart() {
  return reinterpret_cast<size_t>(&__phys_code_start);
}

inline static size_t physDataStart() {
  return reinterpret_cast<size_t>(&__phys_data_start);
}

inline static size_t physHeapStart() {
  return reinterpret_cast<size_t>(&__phys_heap_start);
}

inline static size_t physBootCoreStackStart() {
  return reinterpret_cast<size_t>(&__phys_boot_core_stack_start);
}
}

bool isValidCodeAddress(size_t address);
bool isValidCurrentStackAddress(size_t address);
} // namespace Memory
