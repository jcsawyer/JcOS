#pragma once

#include "../bcm/bcm2xxx_gpio.hpp"
#include "../bcm/bcm2xxx_i2c.hpp"
#include <display/display.hpp>
#include <driver/driver.hpp>
#include <optional.hpp>
#include <stdint.h>

namespace Driver::BSP::Touch {

struct Sample {
  bool active = false;
  uint16_t x = 0;
  uint16_t y = 0;
  uint8_t contactCount = 0;
};

class Controller {
public:
  virtual ~Controller() = default;
  virtual const char *name() const = 0;
  virtual bool probe() = 0;
  virtual bool configure() = 0;
  virtual Optional<Sample> readSample() = 0;
};

class TouchPanel : public Driver::DeviceDriver {
public:
  TouchPanel(Driver::BSP::BCM::GPIO *gpio, Driver::BSP::BCM::I2C *i2c)
      : gpio(gpio), i2c(i2c) {}

  const char *compatible() override { return "Capacitive touch panel"; }
  void init() override;
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;

  bool isReady() const { return ready; }
  Driver::Display::Display::Rotation rotation() const { return panelRotation; }
  void setRotation(Driver::Display::Display::Rotation rotation) {
    panelRotation = rotation;
  }
  bool hasPendingSample() const;
  Optional<Sample> readSample();
  unsigned int rawWidth() const { return sensorWidth; }
  unsigned int rawHeight() const { return sensorHeight; }

private:
  static constexpr unsigned int resetPin = 22;
  static constexpr unsigned int interruptPin = 23;
  static constexpr uint8_t primaryAddress = 0x38;
  static constexpr unsigned int sensorWidth = 480;
  static constexpr unsigned int sensorHeight = 320;

  Driver::BSP::BCM::GPIO *gpio;
  Driver::BSP::BCM::I2C *i2c;
  Driver::Display::Display::Rotation panelRotation =
      Driver::Display::Display::Rotation::Portrait;
  bool ready = false;
  Controller *controller = nullptr;

  void resetController() const;
};

} // namespace Driver::BSP::Touch
