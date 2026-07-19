#include "touch_controller.hpp"

#include <arch/time.hpp>
#include <print.hpp>
#include <time/duration.hpp>

namespace Driver::BSP::Touch {
namespace {

class Ft6x06Controller : public Controller {
public:
  explicit Ft6x06Controller(Driver::BSP::BCM::I2C *bus) : bus(bus) {}

  const char *name() const override { return "FT6x06-compatible"; }

  bool configure() override {
    return bus->writeRegister(address, 0x88, 0x01) &&
           bus->writeRegister(address, 0xA4, 0x01);
  }

  Optional<Sample> readSample() override {
    uint8_t data[5] = {};
    if (!bus->readRegisters(address, 0x02, data, sizeof(data))) {
      return Optional<Sample>();
    }

    Sample sample{};
    sample.contactCount = static_cast<uint8_t>(data[0] & 0x0F);
    sample.active = sample.contactCount > 0;
    if (!sample.active) {
      return Optional<Sample>(sample);
    }

    sample.x = static_cast<uint16_t>(((data[1] & 0x0F) << 8) | data[2]);
    sample.y = static_cast<uint16_t>(((data[3] & 0x0F) << 8) | data[4]);
    return Optional<Sample>(sample);
  }

private:
  Driver::BSP::BCM::I2C *bus;
  static constexpr uint8_t address = 0x38;
};

} // namespace

void TouchPanel::init() {
  ready = false;
  controller = nullptr;

  gpio->configureOutput(resetPin);
  gpio->write(resetPin, true);
  gpio->configureInput(interruptPin);
  gpio->configurePull(interruptPin, Driver::BSP::BCM::GPIO::PullMode::PullUp);
  gpio->configureFallingEdgeDetect(interruptPin, true);
  gpio->clearEventDetect(interruptPin);

  resetController();

  static Ft6x06Controller ft6x06(i2c);
}

void TouchPanel::registerAndEnableIrqHandler(
    ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) {
  (void)irqNumber;
}

bool TouchPanel::hasPendingSample() const {
  if (!ready) {
    return false;
  }

  if (gpio->eventDetected(interruptPin)) {
    gpio->clearEventDetect(interruptPin);
    return true;
  }

  return false;
}

Optional<Sample> TouchPanel::readSample() {
  if (!ready || controller == nullptr) {
    return Optional<Sample>();
  }

  return controller->readSample();
}

void TouchPanel::resetController() const {
  gpio->write(resetPin, false);
  Time::Arch::spinFor(Time::Duration::from_millis(5));
  gpio->write(resetPin, true);
  Time::Arch::spinFor(Time::Duration::from_millis(150));
}

} // namespace Driver::BSP::Touch
