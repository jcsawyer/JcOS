#pragma once

#include "../../../driver/driver.hpp"
#include <stdint.h>

namespace Driver::BSP::LCD {

class LCD16x2 : public Driver::DeviceDriver {
public:
  LCD16x2(unsigned int mmio_start_addr) : registerBlock(mmio_start_addr) {};
  const char *compatible() override { return "LCD 16x2"; }
  void init() override;
  void clear() const;
  void writeChar(char c) const;
  void writeString(const char *str) const;
  void setCursor(unsigned char row, unsigned char col) const;

private:
  void sendCommand(unsigned char cmd) const;
  void sendData(unsigned char data) const;
  void write4Bits(unsigned char nibble) const;
  void pulseEnable() const;
  void delayMicroseconds(unsigned int us) const;
  class RegisterBlock {
  public:
    volatile unsigned int *LCD_DATA;
    volatile unsigned int *LCD_DATA_DDR;
    volatile unsigned int *LCD_RS;
    volatile unsigned int *LCD_EN;

    RegisterBlock(unsigned int mmio_start_addr) {
      LCD_DATA =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x00);
      LCD_DATA_DDR =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x08);
      LCD_RS =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x0C);
      LCD_EN =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x10);
    }
  };
  RegisterBlock registerBlock;
};
} // namespace Driver::BSP::LCD
