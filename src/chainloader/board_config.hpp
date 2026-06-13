#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Chainloader::Board {

#if BOARD == bsp_rpi3
inline constexpr uintptr_t MMIO_BASE = 0x3F000000;
#elif BOARD == bsp_rpi4
inline constexpr uintptr_t MMIO_BASE = 0xFE000000;
#else
#error Unsupported board configuration
#endif

inline constexpr uintptr_t GPIO_BASE = MMIO_BASE + 0x00200000;
inline constexpr uintptr_t UART_BASE = MMIO_BASE + 0x00201000;

inline constexpr uintptr_t PAYLOAD_ENTRY_ADDRESS = 0x00080000;
inline constexpr uintptr_t PAYLOAD_SCRATCH_ADDRESS = 0x01000000;
inline constexpr size_t MAX_PAYLOAD_SIZE = 0x00800000;

inline constexpr unsigned int LCD_COLUMNS = 16;
inline constexpr unsigned int LCD_ROWS = 2;

inline constexpr unsigned char LCD_RS_PIN = 11;
inline constexpr unsigned char LCD_EN_PIN = 10;
inline constexpr unsigned char LCD_D4_PIN = 4;
inline constexpr unsigned char LCD_D5_PIN = 5;
inline constexpr unsigned char LCD_D6_PIN = 6;
inline constexpr unsigned char LCD_D7_PIN = 7;

inline constexpr uint32_t UART_IBRD = 3;
inline constexpr uint32_t UART_FBRD = 16;

inline constexpr uint64_t HOST_REQUEST_TIMEOUT_US = 250000;
inline constexpr uint64_t HOST_RETRY_DELAY_US = 250000;
inline constexpr uint64_t PAYLOAD_IO_TIMEOUT_US = 2000000;
inline constexpr uint64_t RETRY_NOTICE_DELAY_MS = 1000;

} // namespace Chainloader::Board
