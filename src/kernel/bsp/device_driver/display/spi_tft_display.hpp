#pragma once

#include <bsp/device_driver/bcm/bcm2xxx_gpio.hpp>
#include <bsp/device_driver/bcm/bcm2xxx_spi.hpp>
#include <bsp/exception/asynchronous.hpp>
#include <display/display.hpp>
#include <spi/spi.hpp>
#include <stdint.h>

namespace Driver::BSP::Display {

struct TftPanelInitCommand {
  uint8_t command;
  const uint8_t *data;
  size_t dataLength;
  unsigned int postDelayMillis;
};

struct TftPanelConfig {
  unsigned int width;
  unsigned int height;
  unsigned int xOffset;
  unsigned int yOffset;
  bool colorOrderBgr;
  bool invertColors;
  Driver::Display::Rotation defaultRotation;
  const TftPanelInitCommand *initSequence;
  size_t initSequenceLength;
};

class SPITFTDisplay : public Driver::Display::PixelDisplay {
public:
  SPITFTDisplay(Driver::BSP::BCM::SPI *spi, Driver::BSP::BCM::GPIO *gpio,
                const Driver::SPI::DeviceConfig &deviceConfig,
                const Driver::SPI::ControlPins &controlPins,
                const TftPanelConfig &panelConfig);

  const char *compatible() override { return "SPI TFT Display"; }
  void init() override;
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;

  unsigned int width() const override { return currentWidth; }
  unsigned int height() const override { return currentHeight; }
  void setRotation(Driver::Display::Rotation rotation) override;
  void fillScreen(uint16_t color) override;
  void fillRect(unsigned int x, unsigned int y, unsigned int width,
                unsigned int height, uint16_t color) override;
  void blitRgb565(unsigned int x, unsigned int y, unsigned int width,
                  unsigned int height, const uint16_t *pixels) override;

private:
  static constexpr uint8_t COMMAND_SWRESET = 0x01;
  static constexpr uint8_t COMMAND_SLPOUT = 0x11;
  static constexpr uint8_t COMMAND_DISPON = 0x29;
  static constexpr uint8_t COMMAND_CASET = 0x2A;
  static constexpr uint8_t COMMAND_PASET = 0x2B;
  static constexpr uint8_t COMMAND_RAMWR = 0x2C;
  static constexpr uint8_t COMMAND_INVOFF = 0x20;
  static constexpr uint8_t COMMAND_INVON = 0x21;
  static constexpr uint8_t COMMAND_MADCTL = 0x36;
  static constexpr uint8_t COMMAND_COLMOD = 0x3A;

  Driver::SPI::Device spiDevice;
  Driver::BSP::BCM::GPIO *gpio;
  TftPanelConfig panelConfig;
  Driver::Display::Rotation rotation;
  unsigned int currentWidth;
  unsigned int currentHeight;

  void configureControlPins() const;
  void runInitSequence() const;
  void sendCommand(uint8_t command) const;
  void sendCommand(uint8_t command, const uint8_t *data, size_t length) const;
  void sendData(const uint8_t *data, size_t length) const;
  void setAddressWindow(unsigned int x, unsigned int y, unsigned int width,
                        unsigned int height) const;
  void writeRepeatedColor(uint16_t color, size_t pixels) const;
  unsigned int nativeWidth() const;
  unsigned int nativeHeight() const;
};

} // namespace Driver::BSP::Display
