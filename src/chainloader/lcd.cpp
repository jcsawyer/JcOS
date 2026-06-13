#include "lcd.hpp"

#include "board_config.hpp"
#include "timer.hpp"

namespace Chainloader {

namespace {
constexpr unsigned char LCD_CLEARDISPLAY = 0x01;
constexpr unsigned char LCD_DISPLAYOFF = 0x00;
constexpr unsigned char LCD_RETURNHOME = 0x02;
constexpr unsigned char LCD_ENTRYMODESET = 0x04;
constexpr unsigned char LCD_DISPLAYCONTROL = 0x08;
constexpr unsigned char LCD_FUNCTIONSET = 0x20;
constexpr unsigned char LCD_SETDDRAMADDR = 0x80;

constexpr unsigned char LCD_ENTRYLEFT = 0x02;
constexpr unsigned char LCD_DISPLAYON = 0x04;
constexpr unsigned char LCD_4BITMODE = 0x00;
constexpr unsigned char LCD_2LINE = 0x08;
constexpr unsigned char LCD_5x8DOTS = 0x00;
} // namespace

LCD::LCD(GPIO &gpio) : gpio_(gpio) {}

void LCD::init() {
  gpio_.configureOutput(Board::LCD_RS_PIN);
  gpio_.configureOutput(Board::LCD_EN_PIN);
  gpio_.configureOutput(Board::LCD_D4_PIN);
  gpio_.configureOutput(Board::LCD_D5_PIN);
  gpio_.configureOutput(Board::LCD_D6_PIN);
  gpio_.configureOutput(Board::LCD_D7_PIN);

  gpio_.write(Board::LCD_RS_PIN, false);
  gpio_.write(Board::LCD_EN_PIN, false);
  gpio_.write(Board::LCD_D4_PIN, false);
  gpio_.write(Board::LCD_D5_PIN, false);
  gpio_.write(Board::LCD_D6_PIN, false);
  gpio_.write(Board::LCD_D7_PIN, false);

  Timer::delayMillis(50);

  write4Bits(0x03);
  Timer::delayMillis(5);
  write4Bits(0x03);
  Timer::delayMillis(5);
  write4Bits(0x03);
  Timer::delayMillis(5);
  write4Bits(0x02);
  Timer::delayMillis(2);

  sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAYOFF);
  sendCommand(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
  sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAYON);
  sendCommand(LCD_RETURNHOME);
  Timer::delayMillis(2);
  clear();
  sendCommand(LCD_ENTRYMODESET | LCD_ENTRYLEFT);
  sendCommand(LCD_DISPLAYCONTROL | LCD_DISPLAYON);
}

void LCD::clear() {
  sendCommand(LCD_CLEARDISPLAY);
  Timer::delayMillis(2);
}

void LCD::setCursor(unsigned char row, unsigned char column) {
  static const unsigned char rowOffsets[] = {0x00, 0x40};
  sendCommand(LCD_SETDDRAMADDR | (rowOffsets[row] + column));
}

void LCD::writeChar(char value) { sendData(static_cast<unsigned char>(value)); }

void LCD::writeString(const char *text) {
  while (*text != '\0') {
    writeChar(*text++);
  }
}

void LCD::sendCommand(unsigned char command) {
  gpio_.write(Board::LCD_RS_PIN, false);
  write4Bits(static_cast<unsigned char>(command >> 4));
  write4Bits(static_cast<unsigned char>(command & 0x0F));
  Timer::delayMicros(20);
}

void LCD::sendData(unsigned char data) {
  gpio_.write(Board::LCD_RS_PIN, true);
  write4Bits(static_cast<unsigned char>(data >> 4));
  write4Bits(static_cast<unsigned char>(data & 0x0F));
}

void LCD::write4Bits(unsigned char nibble) {
  gpio_.write(Board::LCD_D4_PIN, ((nibble >> 0) & 0x01u) != 0);
  gpio_.write(Board::LCD_D5_PIN, ((nibble >> 1) & 0x01u) != 0);
  gpio_.write(Board::LCD_D6_PIN, ((nibble >> 2) & 0x01u) != 0);
  gpio_.write(Board::LCD_D7_PIN, ((nibble >> 3) & 0x01u) != 0);
  pulseEnable();
}

void LCD::pulseEnable() {
  gpio_.write(Board::LCD_EN_PIN, false);
  Timer::delayMicros(2);
  gpio_.write(Board::LCD_EN_PIN, true);
  Timer::delayMicros(2);
  gpio_.write(Board::LCD_EN_PIN, false);
  Timer::delayMicros(150);
}

} // namespace Chainloader
