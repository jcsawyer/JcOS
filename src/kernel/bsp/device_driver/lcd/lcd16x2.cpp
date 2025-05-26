#include "lcd16x2.hpp"
#include <time.hpp>
#include <time/duration.hpp>

namespace Driver::BSP::LCD {

// Assume GPIO register interface or abstraction here.
// You should replace with real GPIO control logic.
namespace {
constexpr unsigned char RS = 0;
constexpr unsigned char EN = 1;
constexpr unsigned char D4 = 2;
constexpr unsigned char D5 = 3;
constexpr unsigned char D6 = 4;
constexpr unsigned char D7 = 5;

void setPin(unsigned char pin, bool value) {
  // Replace with GPIO write logic.
}

void delayMicroseconds(unsigned int us) {
  Time::Arch::spinFor(Time::Duration::from_micros(us));
}
} // namespace

void LCD16x2::init() {
  delayMicroseconds(50000); // Wait >40ms after power-on

  // 4-bit mode initialization sequence
  write4Bits(0x03);
  delayMicroseconds(4500);
  write4Bits(0x03);
  delayMicroseconds(4500);
  write4Bits(0x03);
  delayMicroseconds(150);
  write4Bits(0x02); // Set to 4-bit mode

  sendCommand(0x28); // Function set: 4-bit, 2 line, 5x8 dots
  sendCommand(0x0C); // Display ON, cursor OFF
  sendCommand(0x06); // Entry mode: increment cursor
  clear();
}

void LCD16x2::clear() const {
  sendCommand(0x01);
  delayMicroseconds(2000);
}

void LCD16x2::setCursor(unsigned char row, unsigned char col) const {
  static const unsigned char rowOffsets[] = {0x00, 0x40};
  if (row > 1)
    row = 1;
  sendCommand(0x80 | (col + rowOffsets[row]));
}

void LCD16x2::writeChar(char c) const {
  sendData(static_cast<unsigned char>(c));
}

void LCD16x2::writeString(const char *str) const {
  while (*str) {
    writeChar(*str++);
  }
}

void LCD16x2::sendCommand(unsigned char cmd) const {
  setPin(RS, false);
  write4Bits(cmd >> 4);
  write4Bits(cmd & 0x0F);
}

void LCD16x2::sendData(unsigned char data) const {
  setPin(RS, true);
  write4Bits(data >> 4);
  write4Bits(data & 0x0F);
}

void LCD16x2::write4Bits(unsigned char nibble) const {
  setPin(D4, nibble & 0x01);
  setPin(D5, nibble & 0x02);
  setPin(D6, nibble & 0x04);
  setPin(D7, nibble & 0x08);
  pulseEnable();
}

void LCD16x2::pulseEnable() const {
  setPin(EN, false);
  delayMicroseconds(1);
  setPin(EN, true);
  delayMicroseconds(1);
  setPin(EN, false);
  delayMicroseconds(100);
}

void LCD16x2::delayMicroseconds(unsigned int us) const {
  Time::Arch::spinFor(Time::Duration::from_micros(us));
}

} // namespace Driver::BSP::LCD
