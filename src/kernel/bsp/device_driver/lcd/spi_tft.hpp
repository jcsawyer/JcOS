#pragma once

#include "../bcm/bcm2xxx_gpio.hpp"
#include "../bcm/bcm2xxx_spi0.hpp"
#include <display/display.hpp>
#include <stdint.h>

namespace Driver::BSP::LCD {

class SpiTft : public Driver::Display::Display {
public:
  SpiTft(Driver::BSP::BCM::GPIO *gpio, Driver::BSP::BCM::SPI0 *spi)
      : gpio(gpio), spi(spi) {}

  const char *compatible() override { return "SPI TFT LCD"; }
  void init() override;
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;

  bool isReady() const override { return ready; }
  unsigned int width() const override { return panelWidth; }
  unsigned int height() const override { return panelHeight; }
  void clear(uint16_t color) override;
  void setAddressWindow(unsigned int x0, unsigned int y0, unsigned int x1,
                        unsigned int y1) override;
  void writePixels(const uint16_t *pixels, size_t count) override;

private:
  enum class PanelProfile {
    Known240x320,
    Generic480x320,
  };

  Driver::BSP::BCM::GPIO *gpio;
  Driver::BSP::BCM::SPI0 *spi;
  bool ready = false;
  unsigned int panelWidth = 240;
  unsigned int panelHeight = 320;
  PanelProfile panelProfile = PanelProfile::Known240x320;

  static constexpr unsigned int lcdCsPin = 8;
  static constexpr unsigned int lcdResetPin = 24;
  static constexpr unsigned int lcdDcPin = 25;

  void hardwareReset();
  bool probePanel();
  void initializePanel();
  void initializeKnown240x320Panel();
  void initializeGeneric480x320Panel();
  void writeCommand(uint8_t command);
  void writeCommand(uint8_t command, const uint8_t *data, size_t length);
  void readCommand(uint8_t command, uint8_t *data, size_t length,
                   size_t dummyBytes);
};

} // namespace Driver::BSP::LCD
