#pragma once

#include "../../../driver/driver.hpp"
#include <stdint.h>

namespace Driver::BSP::LCD {

class HD44780U : public Driver::DeviceDriver {
public:
  HD44780U(unsigned int mmio_start_addr) : registerBlock(mmio_start_addr) {};
  const char *compatible() override { return "LCD 16x2"; }
  void init() override;
  void initLCD() const;
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
  void delayMilliseconds(unsigned int ms) const;
  void setPin(unsigned char pin, bool high) const;
  void gpioSetOutput(int pin) const;
  class RegisterBlock {
  public:
    volatile unsigned int *GPFSEL0;
    volatile unsigned int *GPFSEL1;
    volatile unsigned int *GPSET;
    volatile unsigned int *GPCLR;

    RegisterBlock(unsigned int mmio_start_addr) {
      GPFSEL0 = reinterpret_cast<volatile unsigned int *>(mmio_start_addr +
                                                          0x00); // GPFSEL0
      GPFSEL1 =
          reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x04);
      GPSET = reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x1C);
      GPCLR = reinterpret_cast<volatile unsigned int *>(mmio_start_addr + 0x28);
    }
  };
  RegisterBlock registerBlock;
};
} // namespace Driver::BSP::LCD
