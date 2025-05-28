#pragma once

#include <driver/driver.hpp>
#include <stdint.h>

namespace Driver::BSP::LCD {
// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

class LCD : public Driver::DeviceDriver {
public:
  virtual void initLcd(unsigned char registerSelect, unsigned char enable,
                       unsigned char d0, unsigned char d1, unsigned char d2,
                       unsigned char d3, unsigned char d4, unsigned char d5,
                       unsigned char d6, unsigned char d7) = 0;
  virtual void clear() = 0;
  virtual void home() = 0;
  virtual void blink() = 0;
  virtual void noBlink() = 0;
  virtual void scrollDisplayLeft() = 0;
  virtual void scrollDisplayRight() = 0;
  virtual void autoScroll() = 0;
  virtual void noAutoScroll() = 0;
  virtual void setCursor(unsigned char row, unsigned char col) = 0;
  virtual void writeChar(char c) = 0;
  virtual void writeString(const char *str) = 0;
  virtual void command(unsigned char cmd) = 0;

protected:
  LCD() = default;
  ~LCD() = default;

private:
  virtual void sendCommand(unsigned char cmd) = 0;
  virtual void sendData(unsigned char data) = 0;
  virtual void write4Bits(unsigned char nibble) = 0;
  virtual void write8Bits(unsigned char byte) = 0;
  virtual void pulseEnable() = 0;
  virtual void setPin(unsigned char pin, bool high) = 0;
  virtual void gpioSetOutput(int pin) = 0;
};
} // namespace Driver::BSP::LCD