#include "hd44780u.hpp"
#include <lcd/lcd.hpp>
#include <time.hpp>
#include <time/duration.hpp>

namespace Driver::BSP::LCD {

void Driver::BSP::LCD::HD44780U::registerAndEnableIrqHandler() {}

void Driver::BSP::LCD::HD44780U::initLcd(unsigned char rs, unsigned char en,
                                         unsigned char d0, unsigned char d1,
                                         unsigned char d2, unsigned char d3,
                                         unsigned char d4, unsigned char d5,
                                         unsigned char d6, unsigned char d7) {
  this->rs = rs;
  this->en = en;
  dataPins[0] = d0;
  dataPins[1] = d1;
  dataPins[2] = d2;
  dataPins[3] = d3;
  dataPins[4] = d4;
  dataPins[5] = d5;
  dataPins[6] = d6;
  dataPins[7] = d7;

  // Set all pins as output
  gpioSetOutput(rs);
  gpioSetOutput(en);
  for (int i = 0; i < 8; i++)
    gpioSetOutput(dataPins[i]);

  // 4 - bit mode initialization sequence
  // TODO Add support for 8-bit mode
  write4Bits(0x03);
  delayMilliseconds(5);
  write4Bits(0x03);
  delayMilliseconds(5);
  write4Bits(0x03);
  delayMilliseconds(5);
  write4Bits(0x02);
  delayMilliseconds(1); // Set to 4-bit mode

  sendCommand(0x28);    // Function set: 4-bit, 2 line, 5x8 dots
  sendCommand(0x0C);    // Display ON, cursor OFF
  sendCommand(0x01);    // Clear display
  delayMilliseconds(2); // Clear command takes 1.64ms
  sendCommand(0x06);    // Entry mode: increment cursor
  clear();

  // sendCommand(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
  // sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF |
  //             LCD_BLINKOFF);
  // sendCommand(LCD_CLEARDISPLAY);
  delayMilliseconds(2);
  // sendCommand(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
}

void HD44780U::clear() {
  sendCommand(LCD_CLEARDISPLAY);
  delayMilliseconds(2);
}

void HD44780U::home() {
  sendCommand(LCD_RETURNHOME);
  delayMilliseconds(2);
}

void HD44780U::blink() {
  sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_BLINKON);
}

void HD44780U::noBlink() {
  sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_BLINKOFF);
}

void HD44780U::scrollDisplayLeft() {
  sendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void HD44780U::scrollDisplayRight() {
  sendCommand(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void HD44780U::autoScroll() {
  sendCommand(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTINCREMENT);
}

void HD44780U::noAutoScroll() {
  sendCommand(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
}

void HD44780U::setCursor(unsigned char row, unsigned char col) {
  static const unsigned int row_offsets[] = {0x00, 0x40};
  sendCommand(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void HD44780U::writeChar(char c) { sendData(static_cast<unsigned char>(c)); }

void HD44780U::writeString(const char *str) {
  while (*str) {
    writeChar(*str++);
  }
}

void HD44780U::command(unsigned char cmd) { sendCommand(cmd); }

void HD44780U::sendCommand(unsigned char cmd) {
  setPin(rs, false);
  write4Bits(cmd >> 4);
  write4Bits(cmd & 0x0F);
  delayMicroseconds(20);
}

void HD44780U::sendData(unsigned char data) {
  setPin(rs, true);
  write4Bits(data >> 4);
  write4Bits(data & 0x0F);
}

void HD44780U::write4Bits(unsigned char nibble) {
  for (int i = 4; i < 8; i++) {
    setPin(dataPins[i], (nibble >> i - 4) & 0x01);
  }
  pulseEnable();
}

void HD44780U::write8Bits(unsigned char byte) {
  for (int i = 0; i < 8; i++) {
    setPin(dataPins[i], (byte >> i) & 0x01);
  }
  pulseEnable();
}

void HD44780U::pulseEnable() {
  setPin(en, false);
  delayMicroseconds(1);
  setPin(en, true);
  delayMicroseconds(1);
  setPin(en, false);
  delayMicroseconds(100);
}

void HD44780U::setPin(unsigned char pin, bool high) {
  if (high) {
    *(registerBlock.GPSET + (pin / 32)) = (1 << (pin % 32));
  } else {
    *(registerBlock.GPCLR + (pin / 32)) = (1 << (pin % 32));
  }
}

void HD44780U::gpioSetOutput(int pin) {
  volatile unsigned int *gpfsel =
      registerBlock.GPFSEL0 + (pin / 10); // each GPFSEL controls 10 pins
  int shift = (pin % 10) * 3;
  unsigned int val = *gpfsel;
  val &= ~(0b111 << shift); // clear the 3 bits for the pin
  val |= (0b001 << shift);  // set to output
  *gpfsel = val;
}

void HD44780U::delayMicroseconds(unsigned int us) const {
  Time::Arch::spinFor(Time::Duration::from_micros(us));
}

void HD44780U::delayMilliseconds(unsigned int ms) const {
  Time::Arch::spinFor(Time::Duration::from_millis(ms));
}

} // namespace Driver::BSP::LCD