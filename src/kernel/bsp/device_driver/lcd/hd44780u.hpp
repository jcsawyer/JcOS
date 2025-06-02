#pragma once

#include "hd44780u.hpp"
#include <bsp/exception/asynchronous.hpp>
#include <lcd/lcd.hpp>
#include <stdint.h>

namespace Driver::BSP::LCD {

class HD44780U : public LCD {
public:
  HD44780U(unsigned int mmio_start_addr) : registerBlock(mmio_start_addr) {};
  const char *compatible() { return "HD44780U LCD"; }
  void init() { initLcd(11, 10, 255, 255, 255, 255, 4, 5, 6, 7); };
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;
  void initLcd(unsigned char registerSelect, unsigned char enable,
               unsigned char d0, unsigned char d1, unsigned char d2,
               unsigned char d3, unsigned char d4, unsigned char d5,
               unsigned char d6, unsigned char d7) override;

  void clear() override;
  void home() override;
  void blink() override;
  void noBlink() override;
  void scrollDisplayLeft() override;
  void scrollDisplayRight() override;
  void autoScroll() override;
  void noAutoScroll() override;
  void setCursor(unsigned char row, unsigned char col) override;
  void writeChar(char c) override;
  void writeString(const char *fmt, ...) override;
  void command(unsigned char cmd) override;

private:
  void sendCommand(unsigned char cmd) override;
  void sendData(unsigned char data) override;
  void write4Bits(unsigned char nibble) override;
  void write8Bits(unsigned char byte) override;
  void pulseEnable() override;

  void setPin(unsigned char pin, bool high) override;
  void gpioSetOutput(int pin) override;

  void delayMicroseconds(unsigned int us) const;
  void delayMilliseconds(unsigned int ms) const;

private:
  unsigned char rs = 0;
  unsigned char en = 0;
  unsigned char dataPins[8] = {};
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