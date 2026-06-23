#include "tft_display.hpp"

#include "board_config.hpp"
#include "timer.hpp"

namespace Chainloader {

namespace {
constexpr uint8_t COMMAND_SWRESET = 0x01;
constexpr uint8_t COMMAND_SLPOUT = 0x11;
constexpr uint8_t COMMAND_DISPON = 0x29;
constexpr uint8_t COMMAND_CASET = 0x2A;
constexpr uint8_t COMMAND_PASET = 0x2B;
constexpr uint8_t COMMAND_RAMWR = 0x2C;
constexpr uint8_t COMMAND_INVON = 0x21;
constexpr uint8_t COMMAND_MADCTL = 0x36;
constexpr uint8_t COMMAND_COLMOD = 0x3A;

constexpr uint8_t PIXEL_FORMAT_16BIT = 0x55;
constexpr uint8_t MADCTL_ROTATION_180_BGR = 0x88;
} // namespace

TFTDisplay::TFTDisplay(SPI &spi, GPIO &gpio)
    : spi_(spi), gpio_(gpio), width_(Board::TFT_WIDTH),
      height_(Board::TFT_HEIGHT) {}

void TFTDisplay::init() {
  gpio_.configureOutput(Board::TFT_DATA_COMMAND_PIN);
  gpio_.configureOutput(Board::TFT_RESET_PIN);
  gpio_.write(Board::TFT_DATA_COMMAND_PIN, true);
  gpio_.write(Board::TFT_RESET_PIN, true);

  spi_.init();
  hardwareReset();

  sendCommand(COMMAND_SWRESET);
  Timer::delayMillis(5);
  sendCommand(0x28);
  sendCommand(COMMAND_COLMOD, &PIXEL_FORMAT_16BIT, 1);
  sendCommand(COMMAND_MADCTL, &MADCTL_ROTATION_180_BGR, 1);
  sendCommand(COMMAND_SLPOUT);
  Timer::delayMillis(120);
  sendCommand(COMMAND_INVON);
  sendCommand(COMMAND_DISPON);
  Timer::delayMillis(20);
  fillScreen(Board::TFT_BACKGROUND);
}

void TFTDisplay::fillScreen(uint16_t color) {
  fillRect(0, 0, width_, height_, color);
}

void TFTDisplay::fillRect(unsigned int x, unsigned int y, unsigned int width,
                          unsigned int height, uint16_t color) {
  if (x >= width_ || y >= height_ || width == 0 || height == 0) {
    return;
  }

  if (x + width > width_) {
    width = width_ - x;
  }

  if (y + height > height_) {
    height = height_ - y;
  }

  setAddressWindow(x, y, width, height);
  writeRepeatedColor(color, static_cast<size_t>(width) * height);
}

void TFTDisplay::hardwareReset() const {
  gpio_.write(Board::TFT_RESET_PIN, false);
  Timer::delayMillis(10);
  gpio_.write(Board::TFT_RESET_PIN, true);
  Timer::delayMillis(120);
}

void TFTDisplay::sendCommand(uint8_t command) const {
  sendCommand(command, nullptr, 0);
}

void TFTDisplay::sendCommand(uint8_t command, const uint8_t *data,
                             size_t length) const {
  gpio_.write(Board::TFT_DATA_COMMAND_PIN, false);
  spi_.write(&command, 1);
  if (length > 0) {
    gpio_.write(Board::TFT_DATA_COMMAND_PIN, true);
    spi_.write(data, length);
  }
}

void TFTDisplay::sendData(const uint8_t *data, size_t length) const {
  gpio_.write(Board::TFT_DATA_COMMAND_PIN, true);
  spi_.write(data, length);
}

void TFTDisplay::setAddressWindow(unsigned int x, unsigned int y,
                                  unsigned int width,
                                  unsigned int height) const {
  const uint16_t xStart = static_cast<uint16_t>(x);
  const uint16_t xEnd = static_cast<uint16_t>(x + width - 1);
  const uint16_t yStart = static_cast<uint16_t>(y);
  const uint16_t yEnd = static_cast<uint16_t>(y + height - 1);

  const uint8_t columnData[] = {
      static_cast<uint8_t>(xStart >> 8), static_cast<uint8_t>(xStart & 0xFF),
      static_cast<uint8_t>(xEnd >> 8), static_cast<uint8_t>(xEnd & 0xFF)};
  const uint8_t rowData[] = {
      static_cast<uint8_t>(yStart >> 8), static_cast<uint8_t>(yStart & 0xFF),
      static_cast<uint8_t>(yEnd >> 8), static_cast<uint8_t>(yEnd & 0xFF)};

  sendCommand(COMMAND_CASET, columnData, sizeof(columnData));
  sendCommand(COMMAND_PASET, rowData, sizeof(rowData));
}

void TFTDisplay::writeRepeatedColor(uint16_t color, size_t pixels) const {
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

} // namespace Chainloader
