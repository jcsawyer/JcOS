#pragma once

#include "gpio.hpp"

namespace Chainloader {

class LCD {
public:
  explicit LCD(GPIO &gpio);

  void init();
  void clear();
  void setCursor(unsigned char row, unsigned char column);
  void writeChar(char value);
  void writeString(const char *text);

private:
  GPIO &gpio_;

  void sendCommand(unsigned char command);
  void sendData(unsigned char data);
  void write4Bits(unsigned char nibble);
  void pulseEnable();
};

} // namespace Chainloader
