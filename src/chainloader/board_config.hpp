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
inline constexpr uintptr_t SPI0_BASE = MMIO_BASE + 0x00204000;

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

inline constexpr uint32_t UART_BAUD_RATE = 921600;
inline constexpr uint32_t UART_IBRD = 3;
inline constexpr uint32_t UART_FBRD = 16;

inline constexpr unsigned char UART_TX_PIN = 14;
inline constexpr unsigned char UART_RX_PIN = 15;

inline constexpr unsigned char SPI0_CE0_PIN = 8;
inline constexpr unsigned char SPI0_MOSI_PIN = 10;
inline constexpr unsigned char SPI0_SCLK_PIN = 11;

inline constexpr unsigned char TFT_RESET_PIN = 24;
inline constexpr unsigned char TFT_DATA_COMMAND_PIN = 25;
inline constexpr unsigned int TFT_WIDTH = 320;
inline constexpr unsigned int TFT_HEIGHT = 480;
inline constexpr uint16_t TFT_BACKGROUND = 0x0000;
inline constexpr uint16_t TFT_FOREGROUND = 0xFFFF;

} // namespace Chainloader::Board
