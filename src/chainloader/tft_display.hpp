#pragma once

#include "gpio.hpp"
#include "spi.hpp"

#include <stddef.h>
#include <stdint.h>

namespace Chainloader {

class TFTDisplay {
public:
  TFTDisplay(SPI &spi, GPIO &gpio);

  void init();
  void fillScreen(uint16_t color);
  void fillRect(unsigned int x, unsigned int y, unsigned int width,
                unsigned int height, uint16_t color);

  unsigned int width() const { return width_; }
  unsigned int height() const { return height_; }

private:
  SPI &spi_;
  GPIO &gpio_;
  unsigned int width_;
  unsigned int height_;

  void hardwareReset() const;
  void sendCommand(uint8_t command) const;
  void sendCommand(uint8_t command, const uint8_t *data, size_t length) const;
  void sendData(const uint8_t *data, size_t length) const;
  void setAddressWindow(unsigned int x, unsigned int y, unsigned int width,
                        unsigned int height) const;
  void writeRepeatedColor(uint16_t color, size_t pixels) const;
};

} // namespace Chainloader
