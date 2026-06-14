#include "bcm2xxx_spi.hpp"

#include <arch/cpu.hpp>
#include <time.hpp>
#include <time/duration.hpp>

namespace Driver::BSP::BCM {

namespace {
uint32_t chipSelectBits(Driver::SPI::ChipSelect chipSelect) {
  switch (chipSelect) {
  case Driver::SPI::ChipSelect::CS0:
    return 0u;
  case Driver::SPI::ChipSelect::CS1:
    return 1u;
  }

  return 0u;
}
} // namespace

void SPI::init() {
  *registerBlock.CLK = 32;
  *registerBlock.DLEN = 0;
  *registerBlock.LTOH = 0;
  *registerBlock.DC = 0;
  clearFifos();
  *registerBlock.CS = buildControlWord(
      {Driver::SPI::Mode::Mode0, 32, Driver::SPI::ChipSelect::CS0,
       Driver::SPI::ChipSelectPolicy::Hardware});
}

void SPI::registerAndEnableIrqHandler(
    ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) {
  (void)irqNumber;
}

void SPI::configure(const Driver::SPI::DeviceConfig &config) const {
  *registerBlock.CLK = config.clockDivider == 0 ? 2 : config.clockDivider;
  *registerBlock.DLEN = 0;
  *registerBlock.CS = buildControlWord(config);
}

void SPI::beginTransfer(const Driver::SPI::DeviceConfig &config) const {
  configure(config);
  *registerBlock.CS = buildControlWord(config) | CS_TA;
}

void SPI::endTransfer(const Driver::SPI::DeviceConfig &config) const {
  while (((*registerBlock.CS) & CS_DONE) == 0) {
    CPU::nop();
  }

  *registerBlock.CS = buildControlWord(config);
}

void SPI::write(const uint8_t *data, size_t length) const {
  for (size_t i = 0; i < length; ++i) {
    while (((*registerBlock.CS) & CS_TXD) == 0) {
      CPU::nop();
    }

    *registerBlock.FIFO = data[i];

    while (((*registerBlock.CS) & CS_RXD) != 0) {
      (void)*registerBlock.FIFO;
    }
  }
}

uint8_t SPI::transfer(uint8_t value) const {
  while (((*registerBlock.CS) & CS_TXD) == 0) {
    CPU::nop();
  }

  *registerBlock.FIFO = value;

  while (((*registerBlock.CS) & CS_RXD) == 0) {
    CPU::nop();
  }

  return static_cast<uint8_t>(*registerBlock.FIFO);
}

uint32_t SPI::buildControlWord(const Driver::SPI::DeviceConfig &config) const {
  uint32_t control = chipSelectBits(config.chipSelect);
  control |= CS_CLEAR_RX | CS_CLEAR_TX | CS_DONE;

  switch (config.mode) {
  case Driver::SPI::Mode::Mode0:
    break;
  case Driver::SPI::Mode::Mode1:
    control |= CS_CPHA;
    break;
  case Driver::SPI::Mode::Mode2:
    control |= CS_CPOL;
    break;
  case Driver::SPI::Mode::Mode3:
    control |= CS_CPOL | CS_CPHA;
    break;
  }

  return control;
}

void SPI::clearFifos() const {
  *registerBlock.CS = CS_CLEAR_RX | CS_CLEAR_TX;
  Time::Arch::spinFor(Time::Duration::from_micros(1));
}

} // namespace Driver::BSP::BCM

namespace Driver::SPI {

Device::Device(Driver::BSP::BCM::SPI *spi, Driver::BSP::BCM::GPIO *gpio,
               const DeviceConfig &config, const ControlPins &pins)
    : spi(spi), gpio(gpio), config(config), pins(pins) {}

void Device::begin() const {
  if (pins.chipSelect.present &&
      config.chipSelectPolicy == ChipSelectPolicy::Gpio) {
    gpio->write(pins.chipSelect.pin, false);
  }

  spi->beginTransfer(config);
}

void Device::end() const {
  spi->endTransfer(config);

  if (pins.chipSelect.present &&
      config.chipSelectPolicy == ChipSelectPolicy::Gpio) {
    gpio->write(pins.chipSelect.pin, true);
  }
}

void Device::write(const uint8_t *data, size_t length) const {
  spi->write(data, length);
}

uint8_t Device::transfer(uint8_t value) const { return spi->transfer(value); }

void Device::setDataMode() const {
  if (pins.dataCommand.present) {
    gpio->write(pins.dataCommand.pin, true);
  }
}

void Device::setCommandMode() const {
  if (pins.dataCommand.present) {
    gpio->write(pins.dataCommand.pin, false);
  }
}

void Device::hardwareReset(unsigned int holdMillis,
                           unsigned int settleMillis) const {
  if (!pins.reset.present) {
    return;
  }

  gpio->write(pins.reset.pin, true);
  Time::Arch::spinFor(Time::Duration::from_millis(settleMillis));
  gpio->write(pins.reset.pin, false);
  Time::Arch::spinFor(Time::Duration::from_millis(holdMillis));
  gpio->write(pins.reset.pin, true);
  Time::Arch::spinFor(Time::Duration::from_millis(settleMillis));
}

} // namespace Driver::SPI
