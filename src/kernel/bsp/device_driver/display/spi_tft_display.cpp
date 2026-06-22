#include "spi_tft_display.hpp"

#include <time.hpp>
#include <time/duration.hpp>

namespace Driver::BSP::Display {

namespace {
uint8_t rotationMadctl(Driver::Display::Rotation rotation, bool colorOrderBgr) {
  uint8_t value = colorOrderBgr ? 0x08 : 0x00;

  switch (rotation) {
  case Driver::Display::Rotation::Deg0:
    return value | 0x40;
  case Driver::Display::Rotation::Deg90:
    return value | 0x20;
  case Driver::Display::Rotation::Deg180:
    return value | 0x80;
  case Driver::Display::Rotation::Deg270:
    return value | 0xE0;
  }

  return value | 0x40;
}
} // namespace

SPITFTDisplay::SPITFTDisplay(Driver::BSP::BCM::SPI *spi,
                             Driver::BSP::BCM::GPIO *gpio,
                             const Driver::SPI::DeviceConfig &deviceConfig,
                             const Driver::SPI::ControlPins &controlPins,
                             const TftPanelConfig &panelConfig)
    : spiDevice(spi, gpio, deviceConfig, controlPins), gpio(gpio),
      panelConfig(panelConfig), rotation(panelConfig.defaultRotation),
      currentWidth(panelConfig.width), currentHeight(panelConfig.height) {}

void SPITFTDisplay::init() {
  configureControlPins();
  spiDevice.hardwareReset(10, 120);
  runInitSequence();
  if (panelConfig.invertColors) {
    sendCommand(COMMAND_INVON);
  } else {
    sendCommand(COMMAND_INVOFF);
  }
  setRotation(panelConfig.defaultRotation);
  fillScreen(0x0000);
}

void SPITFTDisplay::registerAndEnableIrqHandler(
    ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) {
  (void)irqNumber;
}

void SPITFTDisplay::setRotation(Driver::Display::Rotation value) {
  rotation = value;
  const bool swapAxes = value == Driver::Display::Rotation::Deg90 ||
                        value == Driver::Display::Rotation::Deg270;
  currentWidth = swapAxes ? nativeHeight() : nativeWidth();
  currentHeight = swapAxes ? nativeWidth() : nativeHeight();

  const uint8_t madctl = rotationMadctl(rotation, panelConfig.colorOrderBgr);
  sendCommand(COMMAND_MADCTL, &madctl, 1);
}

void SPITFTDisplay::fillScreen(uint16_t color) {
  fillRect(0, 0, width(), height(), color);
}

void SPITFTDisplay::fillRect(unsigned int x, unsigned int y, unsigned int w,
                             unsigned int h, uint16_t color) {
  if (x >= width() || y >= height() || w == 0 || h == 0) {
    return;
  }

  if (x + w > width()) {
    w = width() - x;
  }

  if (y + h > height()) {
    h = height() - y;
  }

  setAddressWindow(x, y, w, h);
  writeRepeatedColor(color, static_cast<size_t>(w) * h);
}

void SPITFTDisplay::blitRgb565(unsigned int x, unsigned int y, unsigned int w,
                               unsigned int h, const uint16_t *pixels) {
  if (pixels == nullptr || x >= width() || y >= height() || w == 0 || h == 0) {
    return;
  }

  if (x + w > width()) {
    w = width() - x;
  }

  if (y + h > height()) {
    h = height() - y;
  }

  setAddressWindow(x, y, w, h);
  sendCommand(COMMAND_RAMWR);

  uint8_t chunk[64];
  size_t chunkLength = 0;
  const size_t totalPixels = static_cast<size_t>(w) * h;
  for (size_t i = 0; i < totalPixels; ++i) {
    const uint16_t pixel = pixels[i];
    chunk[chunkLength++] = static_cast<uint8_t>(pixel >> 8);
    chunk[chunkLength++] = static_cast<uint8_t>(pixel & 0xFF);

    if (chunkLength == sizeof(chunk)) {
      sendData(chunk, chunkLength);
      chunkLength = 0;
    }
  }

  if (chunkLength > 0) {
    sendData(chunk, chunkLength);
  }
}

void SPITFTDisplay::configureControlPins() const {
  const auto pins = spiDevice.controlPins();

  if (pins.dataCommand.present) {
    gpio->setOutput(pins.dataCommand.pin);
    gpio->write(pins.dataCommand.pin, true);
  }

  if (pins.reset.present) {
    gpio->setOutput(pins.reset.pin);
    gpio->write(pins.reset.pin, true);
  }

  if (pins.chipSelect.present && spiDevice.deviceConfig().chipSelectPolicy ==
                                     Driver::SPI::ChipSelectPolicy::Gpio) {
    gpio->setOutput(pins.chipSelect.pin);
    gpio->write(pins.chipSelect.pin, true);
  }
}

void SPITFTDisplay::runInitSequence() const {
  for (size_t i = 0; i < panelConfig.initSequenceLength; ++i) {
    const auto &step = panelConfig.initSequence[i];
    sendCommand(step.command, step.data, step.dataLength);
    if (step.postDelayMillis > 0) {
      Time::Arch::spinFor(Time::Duration::from_millis(step.postDelayMillis));
    }
  }

  const uint8_t pixelFormat = 0x55;
  sendCommand(COMMAND_COLMOD, &pixelFormat, 1);
}

void SPITFTDisplay::sendCommand(uint8_t command) const {
  sendCommand(command, nullptr, 0);
}

void SPITFTDisplay::sendCommand(uint8_t command, const uint8_t *data,
                                size_t length) const {
  spiDevice.begin();
  spiDevice.setCommandMode();
  spiDevice.write(&command, 1);
  if (length > 0) {
    spiDevice.setDataMode();
    spiDevice.write(data, length);
  }
  spiDevice.end();
}

void SPITFTDisplay::sendData(const uint8_t *data, size_t length) const {
  spiDevice.begin();
  spiDevice.setDataMode();
  spiDevice.write(data, length);
  spiDevice.end();
}

void SPITFTDisplay::setAddressWindow(unsigned int x, unsigned int y,
                                     unsigned int w, unsigned int h) const {
  const uint16_t xStart = static_cast<uint16_t>(x + panelConfig.xOffset);
  const uint16_t xEnd = static_cast<uint16_t>(x + w - 1 + panelConfig.xOffset);
  const uint16_t yStart = static_cast<uint16_t>(y + panelConfig.yOffset);
  const uint16_t yEnd = static_cast<uint16_t>(y + h - 1 + panelConfig.yOffset);

  const uint8_t columnData[] = {
      static_cast<uint8_t>(xStart >> 8), static_cast<uint8_t>(xStart & 0xFF),
      static_cast<uint8_t>(xEnd >> 8), static_cast<uint8_t>(xEnd & 0xFF)};
  const uint8_t rowData[] = {
      static_cast<uint8_t>(yStart >> 8), static_cast<uint8_t>(yStart & 0xFF),
      static_cast<uint8_t>(yEnd >> 8), static_cast<uint8_t>(yEnd & 0xFF)};

  sendCommand(COMMAND_CASET, columnData, sizeof(columnData));
  sendCommand(COMMAND_PASET, rowData, sizeof(rowData));
}

void SPITFTDisplay::writeRepeatedColor(uint16_t color, size_t pixels) const {
  uint8_t chunk[64];
  for (size_t i = 0; i < sizeof(chunk); i += 2) {
    chunk[i] = static_cast<uint8_t>(color >> 8);
    chunk[i + 1] = static_cast<uint8_t>(color & 0xFF);
  }

  sendCommand(COMMAND_RAMWR);

  while (pixels > 0) {
    size_t chunkPixels = pixels;
    if (chunkPixels > sizeof(chunk) / 2) {
      chunkPixels = sizeof(chunk) / 2;
    }

    sendData(chunk, chunkPixels * 2);
    pixels -= chunkPixels;
  }
}

unsigned int SPITFTDisplay::nativeWidth() const { return panelConfig.width; }

unsigned int SPITFTDisplay::nativeHeight() const { return panelConfig.height; }

} // namespace Driver::BSP::Display
