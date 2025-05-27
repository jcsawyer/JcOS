#include "hd44780u.hpp"
#include <print.hpp>
#include <time.hpp>
#include <time/duration.hpp>

namespace Driver::BSP::LCD {

constexpr unsigned char RS = 11;
constexpr unsigned char EN = 10;
constexpr unsigned char D4 = 4;
constexpr unsigned char D5 = 5;
constexpr unsigned char D6 = 6;
constexpr unsigned char D7 = 7;

// GPIO 4-7 os connected to display data pins 4-7
// GPIO 11 is the RS pin
// GPIO 10 is the Enable pin
void HD44780U::setPin(unsigned char pin, bool high) const {
  debug("Setting pin %d to %b", pin, high);
  unsigned int reg = pin / 32;
  int shift = pin % 32;
  volatile unsigned int *reg_addr = reinterpret_cast<volatile unsigned int *>(
      *registerBlock.GPFSEL0 +
      ((high ? *registerBlock.GPFSEL0 : *registerBlock.GPCLR) / 4) + reg);
  *reg_addr = (1 << shift);
}

void HD44780U::gpioSetOutput(int pin) const {
  debug("Setting pin %d to OUTPUT", pin);
  int reg = pin / 10;
  int shift = (pin % 10) * 3;
  volatile unsigned int *fsel = reinterpret_cast<volatile unsigned int *>(
      *registerBlock.GPFSEL0 + (*registerBlock.GPFSEL0 / 4) + reg);

  unsigned int val = *fsel;
  val &= ~(0b111 << shift); // clear
  val |= (0b001 << shift);  // output
  *fsel = val;
}

void delayMicroseconds(unsigned int us) {
  Time::Arch::spinFor(Time::Duration::from_micros(us));
}

void HD44780U::init() {
  delayMicroseconds(200); // Wait >40ms after power-on

  // Set pins as outputs
  gpioSetOutput(RS);
  gpioSetOutput(EN);
  gpioSetOutput(D4);
  gpioSetOutput(D5);
  gpioSetOutput(D6);
  gpioSetOutput(D7);
}

void HD44780U::initLCD() const {
  // 4-bit mode initialization sequence
  write4Bits(0x03);
  delayMicroseconds(200);
  write4Bits(0x03);
  delayMicroseconds(50);
  write4Bits(0x03);
  delayMicroseconds(150);
  write4Bits(0x02); // Set to 4-bit mode

  sendCommand(0x28); // Function set: 4-bit, 2 line, 5x8 dots
  sendCommand(0x0C); // Display ON, cursor OFF
  sendCommand(0x06); // Entry mode: increment cursor
  clear();
}

void HD44780U::clear() const {
  sendCommand(0x01);
  delayMicroseconds(2000);
}

void HD44780U::setCursor(unsigned char row, unsigned char col) const {
  static const unsigned char rowOffsets[] = {0x00, 0x40};
  if (row > 1)
    row = 1;
  sendCommand(0x80 | (col + rowOffsets[row]));
}

void HD44780U::writeChar(char c) const {
  sendData(static_cast<unsigned char>(c));
}

void HD44780U::writeString(const char *str) const {
  while (*str) {
    writeChar(*str++);
  }
}

void HD44780U::sendCommand(unsigned char cmd) const {
  setPin(RS, false);
  write4Bits(cmd >> 4);
  write4Bits(cmd & 0x0F);
  delayMicroseconds(20);
}

void HD44780U::sendData(unsigned char data) const {
  setPin(RS, true);
  write4Bits(data >> 4);
  write4Bits(data & 0x0F);
}

void HD44780U::write4Bits(unsigned char nibble) const {
  setPin(D4, nibble & 0x01);
  setPin(D5, nibble & 0x02);
  setPin(D6, nibble & 0x04);
  setPin(D7, nibble & 0x08);
  pulseEnable();
}

void HD44780U::pulseEnable() const {
  setPin(EN, false);
  delayMicroseconds(1);
  setPin(EN, true);
  delayMicroseconds(1);
  setPin(EN, false);
  delayMicroseconds(100);
}

void HD44780U::delayMicroseconds(unsigned int us) const {
  Time::Arch::spinFor(Time::Duration::from_millis(us));
}

} // namespace Driver::BSP::LCD
