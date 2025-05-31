#pragma once
#include <stddef.h>

#include <print.hpp>

//! BSP Memory Management.
//!
//! The physical memory layout.
//!
//! The Raspberry's firmware copies the kernel binary to 0x8_0000. The preceding
//! region will be used as the boot core's stack.
//!
//! +---------------------------------------+
//! |                                       | 0x0
//! |                                       |                                ^
//! | Boot-core Stack                       |                                |
//! stack |                                       | | growth | | | direction
//! +---------------------------------------+
//! |                                       | code_start @ 0x8_0000
//! | .text                                 |
//! | .rodata                               |
//! | .got                                  |
//! |                                       |
//! +---------------------------------------+
//! |                                       | code_end_exclusive
//! | .data                                 |
//! | .bss                                  |
//! |                                       |
//! +---------------------------------------+
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
  size_t END_INCLUSIVE;

  MMIO(size_t start, size_t videocore_mbox_start, size_t rng_start,
       size_t gpio_start, size_t uart_start, size_t timer_start,
       size_t end_inclusive)
      : START(start), VIDEOCORE_MBOX_START(videocore_mbox_start),
        RNG_START(rng_start), GPIO_START(gpio_start),
        PL011_UART_START(uart_start), TIMER_START(timer_start),
        END_INCLUSIVE(end_inclusive) {}
};
struct Map {
  /// The inclusive end address of the memory map.
  ///
  /// End address + 1 must be power of two.
  ///
  /// # Note
  ///
  /// RPi3 and RPi4 boards can have different amounts of RAM. To make our code
  /// lean for educational purposes, we set the max size of the address space to
  /// 4 GiB regardless of board. This way, we can map the entire range that we
  /// need (end of MMIO for RPi4) in one take.
  ///
  /// However, making this trade-off has the downside of making it possible for
  /// the CPU to assert a physical address that is not backed by any DRAM (e.g.
  /// accessing an address close to 4 GiB on an RPi3 that comes with 1 GiB of
  /// RAM). This would result in a crash or other kind of error.
  static const size_t END_INCLUSIVE = 0xFFFFFFFF;

  static const size_t BOARD_DEFAULT_LOAD_ADDRESS = 0x80000;

  static const size_t VIDEOCORE_MBOX_OFFSET = 0x0000B880;
  static const size_t RNG_OFFSET = 0x00104000;
  static const size_t GPIO_OFFSET = 0x00200000;
  static const size_t UART_OFFSET = 0x00201000;
  static const size_t TIMER_OFFSET = 0x00003000;

  inline static MMIO getMMIO() {
#if BOARD == bsp_rpi3
    return MMIO(0x3F000000, 0x3F000000 + VIDEOCORE_MBOX_OFFSET,
                0x3F000000 + RNG_OFFSET, 0x3F000000 + GPIO_OFFSET,
                0x3F000000 + UART_OFFSET, 0x3F000000 + TIMER_OFFSET,
                0x4000FFFF);
#elif BOARD == bsp_rpi4
    return MMIO(0xFE000000, 0xFE000000 + VIDEOCORE_MBOX_OFFSET,
                0xFE000000 + RNG_OFFSET, 0xFE000000 + GPIO_OFFSET,
                0xFE000000 + UART_OFFSET, 0xFE000000 + TIMER_OFFSET,
                0xFF84FFFF);
#else
#error Unknown board
#endif
  }
};

extern "C" {
extern size_t __code_start;
extern size_t __code_end_exclusive;

inline static size_t codeStart() {
  // The start address of the code segment is provided by the linker script.
  return reinterpret_cast<size_t>(&__code_start);
}

inline static size_t codeEndExclusive() {
  // The end address of the code segment (exclusive) is provided by the
  // linker script.
  return reinterpret_cast<size_t>(&__code_end_exclusive);
}
}
} // namespace Memory
