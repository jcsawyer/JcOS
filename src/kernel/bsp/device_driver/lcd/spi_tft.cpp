#include "spi_tft.hpp"

#include <print.hpp>
#include <time/duration.hpp>

#include <arch/time.hpp>

namespace Driver::BSP::LCD {

namespace {
constexpr uint16_t colorBlack = 0x0000;
constexpr uint8_t commandSoftwareReset = 0x01;
constexpr uint8_t commandSleepOut = 0x11;
constexpr uint8_t commandDisplayOn = 0x29;
constexpr uint8_t commandColumnAddressSet = 0x2A;
constexpr uint8_t commandPageAddressSet = 0x2B;
constexpr uint8_t commandMemoryWrite = 0x2C;
constexpr uint8_t commandPixelFormat = 0x3A;
constexpr uint8_t commandMemoryAccessControl = 0x36;
constexpr uint8_t commandReadDisplayId = 0x04;
constexpr uint8_t commandReadId4 = 0xD3;
} // namespace

void SpiTft::init() {
  ready = false;
  panelProfile = PanelProfile::Known240x320;
  panelWidth = 240;
  panelHeight = 320;

  gpio->mapSpi0();
  gpio->configureOutput(lcdResetPin);
  gpio->configureOutput(lcdDcPin);
  gpio->write(lcdResetPin, true);
  gpio->write(lcdDcPin, true);

  spi->configure(64);

  hardwareReset();

  if (!probePanel()) {
    panelProfile = PanelProfile::Generic480x320;
    panelWidth = 480;
    panelHeight = 320;
    warn("SPI TFT probe did not match the known 240x320 profile; trying "
         "generic 480x320 SPI TFT init");
  }

  initializePanel();
  ready = true;
  clear(colorBlack);
  info("TFT initialized at %ux%u", panelWidth, panelHeight);
}

void SpiTft::registerAndEnableIrqHandler(
    ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) {
  (void)irqNumber;
}

void SpiTft::clear(uint16_t color) {
  if (!ready) {
    return;
  }

  setAddressWindow(0, 0, panelWidth - 1, panelHeight - 1);
  constexpr size_t chunkPixels = 64;
  uint16_t chunk[chunkPixels];
  for (size_t i = 0; i < chunkPixels; i++) {
    chunk[i] = color;
  }

  size_t remaining = static_cast<size_t>(panelWidth) * panelHeight;
  while (remaining > 0) {
    const size_t currentChunk =
        remaining > chunkPixels ? chunkPixels : remaining;
    writePixels(chunk, currentChunk);
    remaining -= currentChunk;
  }
}

void SpiTft::setAddressWindow(unsigned int x0, unsigned int y0, unsigned int x1,
                              unsigned int y1) {
  const uint8_t columnAddress[] = {
      static_cast<uint8_t>(x0 >> 8), static_cast<uint8_t>(x0 & 0xFF),
      static_cast<uint8_t>(x1 >> 8), static_cast<uint8_t>(x1 & 0xFF)};
  const uint8_t pageAddress[] = {
      static_cast<uint8_t>(y0 >> 8), static_cast<uint8_t>(y0 & 0xFF),
      static_cast<uint8_t>(y1 >> 8), static_cast<uint8_t>(y1 & 0xFF)};

  writeCommand(commandColumnAddressSet, columnAddress, sizeof(columnAddress));
  writeCommand(commandPageAddressSet, pageAddress, sizeof(pageAddress));
  writeCommand(commandMemoryWrite);
}

void SpiTft::writePixels(const uint16_t *pixels, size_t count) {
  if (pixels == nullptr || count == 0) {
    return;
  }

  spi->beginTransaction();
  gpio->write(lcdDcPin, true);
  for (size_t i = 0; i < count; i++) {
    spi->transferByte(static_cast<uint8_t>(pixels[i] >> 8));
    spi->transferByte(static_cast<uint8_t>(pixels[i] & 0xFF));
  }
  spi->endTransaction();
}

void SpiTft::hardwareReset() {
  gpio->write(lcdResetPin, true);
  Time::Arch::spinFor(Time::Duration::from_millis(5));
  gpio->write(lcdResetPin, false);
  Time::Arch::spinFor(Time::Duration::from_millis(20));
  gpio->write(lcdResetPin, true);
  Time::Arch::spinFor(Time::Duration::from_millis(150));
}

bool SpiTft::probePanel() {
  uint8_t id4[3] = {};
  readCommand(commandReadId4, id4, sizeof(id4), 1);
  info("TFT ID4 bytes: %02x %02x %02x", id4[0], id4[1], id4[2]);

  if (id4[1] == 0x93 && id4[2] == 0x41) {
    return true;
  }

  uint8_t displayId[3] = {};
  readCommand(commandReadDisplayId, displayId, sizeof(displayId), 1);
  info("TFT display ID bytes: %02x %02x %02x", displayId[0], displayId[1],
       displayId[2]);

  return displayId[1] == 0x93 && displayId[2] == 0x41;
}

void SpiTft::initializePanel() {
  switch (panelProfile) {
  case PanelProfile::Known240x320:
    initializeKnown240x320Panel();
    break;
  case PanelProfile::Generic480x320:
    initializeGeneric480x320Panel();
    break;
  }
}

void SpiTft::initializeKnown240x320Panel() {
  writeCommand(commandSoftwareReset);
  Time::Arch::spinFor(Time::Duration::from_millis(150));

  static const uint8_t ef[] = {0x03, 0x80, 0x02};
  static const uint8_t cf[] = {0x00, 0xC1, 0x30};
  static const uint8_t ed[] = {0x64, 0x03, 0x12, 0x81};
  static const uint8_t e8[] = {0x85, 0x00, 0x78};
  static const uint8_t cb[] = {0x39, 0x2C, 0x00, 0x34, 0x02};
  static const uint8_t f7[] = {0x20};
  static const uint8_t ea[] = {0x00, 0x00};
  static const uint8_t c0[] = {0x23};
  static const uint8_t c1[] = {0x10};
  static const uint8_t c5[] = {0x3E, 0x28};
  static const uint8_t c7[] = {0x86};
  static const uint8_t madctl[] = {0x48};
  static const uint8_t pixelFormat[] = {0x55};
  static const uint8_t b1[] = {0x00, 0x18};
  static const uint8_t b6[] = {0x08, 0x82, 0x27};
  static const uint8_t f2[] = {0x00};
  static const uint8_t gammaSet[] = {0x01};
  static const uint8_t e0[] = {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                               0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00};
  static const uint8_t e1[] = {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                               0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F};

  writeCommand(0xEF, ef, sizeof(ef));
  writeCommand(0xCF, cf, sizeof(cf));
  writeCommand(0xED, ed, sizeof(ed));
  writeCommand(0xE8, e8, sizeof(e8));
  writeCommand(0xCB, cb, sizeof(cb));
  writeCommand(0xF7, f7, sizeof(f7));
  writeCommand(0xEA, ea, sizeof(ea));
  writeCommand(0xC0, c0, sizeof(c0));
  writeCommand(0xC1, c1, sizeof(c1));
  writeCommand(0xC5, c5, sizeof(c5));
  writeCommand(0xC7, c7, sizeof(c7));
  writeCommand(commandMemoryAccessControl, madctl, sizeof(madctl));
  writeCommand(commandPixelFormat, pixelFormat, sizeof(pixelFormat));
  writeCommand(0xB1, b1, sizeof(b1));
  writeCommand(0xB6, b6, sizeof(b6));
  writeCommand(0xF2, f2, sizeof(f2));
  writeCommand(0x26, gammaSet, sizeof(gammaSet));
  writeCommand(0xE0, e0, sizeof(e0));
  writeCommand(0xE1, e1, sizeof(e1));
  writeCommand(commandSleepOut);
  Time::Arch::spinFor(Time::Duration::from_millis(120));
  writeCommand(commandDisplayOn);
  Time::Arch::spinFor(Time::Duration::from_millis(20));
}

void SpiTft::initializeGeneric480x320Panel() {
  writeCommand(commandSoftwareReset);
  Time::Arch::spinFor(Time::Duration::from_millis(150));

  static const uint8_t powerControl1[] = {0x0D, 0x0D};
  static const uint8_t powerControl2[] = {0x43, 0x00};
  static const uint8_t powerControl3[] = {0x00};
  static const uint8_t vcomControl[] = {0x00, 0x48};
  static const uint8_t memoryAccessControl[] = {0x28};
  static const uint8_t pixelFormat[] = {0x55};
  static const uint8_t frameControl[] = {0xB0, 0x11};
  static const uint8_t displayInversion[] = {0x02};
  static const uint8_t displayFunction[] = {0x02, 0x02, 0x3B};
  static const uint8_t entryMode[] = {0xC6};
  static const uint8_t gammaPositive[] = {0x0F, 0x1F, 0x1C, 0x0C, 0x0F,
                                          0x08, 0x48, 0x98, 0x37, 0x0A,
                                          0x13, 0x04, 0x11, 0x0D, 0x00};
  static const uint8_t gammaNegative[] = {0x0F, 0x32, 0x2E, 0x0B, 0x0D,
                                          0x05, 0x47, 0x75, 0x37, 0x06,
                                          0x10, 0x03, 0x24, 0x20, 0x00};

  writeCommand(commandSleepOut);
  Time::Arch::spinFor(Time::Duration::from_millis(120));
  writeCommand(0xC0, powerControl1, sizeof(powerControl1));
  writeCommand(0xC1, powerControl2, sizeof(powerControl2));
  writeCommand(0xC2, powerControl3, sizeof(powerControl3));
  writeCommand(0xC5, vcomControl, sizeof(vcomControl));
  writeCommand(commandMemoryAccessControl, memoryAccessControl,
               sizeof(memoryAccessControl));
  writeCommand(commandPixelFormat, pixelFormat, sizeof(pixelFormat));
  writeCommand(0xB1, frameControl, sizeof(frameControl));
  writeCommand(0xB4, displayInversion, sizeof(displayInversion));
  writeCommand(0xB6, displayFunction, sizeof(displayFunction));
  writeCommand(0xB7, entryMode, sizeof(entryMode));
  writeCommand(0xE0, gammaPositive, sizeof(gammaPositive));
  writeCommand(0xE1, gammaNegative, sizeof(gammaNegative));
  writeCommand(commandDisplayOn);
  Time::Arch::spinFor(Time::Duration::from_millis(20));
}

void SpiTft::writeCommand(uint8_t command) {
  spi->beginTransaction();
  gpio->write(lcdDcPin, false);
  spi->transferByte(command);
  spi->endTransaction();
}

void SpiTft::writeCommand(uint8_t command, const uint8_t *data, size_t length) {
  spi->beginTransaction();
  gpio->write(lcdDcPin, false);
  spi->transferByte(command);
  gpio->write(lcdDcPin, true);
  spi->write(data, length);
  spi->endTransaction();
}

void SpiTft::readCommand(uint8_t command, uint8_t *data, size_t length,
                         size_t dummyBytes) {
  spi->beginTransaction();
  gpio->write(lcdDcPin, false);
  spi->transferByte(command);
  gpio->write(lcdDcPin, true);

  for (size_t i = 0; i < dummyBytes; i++) {
    (void)spi->transferByte(0x00);
  }

  for (size_t i = 0; i < length; i++) {
    data[i] = spi->transferByte(0x00);
  }

  spi->endTransaction();
}

} // namespace Driver::BSP::LCD
