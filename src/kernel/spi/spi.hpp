#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Driver::BSP::BCM {
class GPIO;
class SPI;
} // namespace Driver::BSP::BCM

namespace Driver::SPI {

struct ControlPin {
  bool present;
  unsigned int pin;

  static constexpr ControlPin none() { return {false, 0}; }
  static constexpr ControlPin gpio(unsigned int value) { return {true, value}; }
};

struct ControlPins {
  ControlPin chipSelect;
  ControlPin dataCommand;
  ControlPin reset;
  ControlPin irq;
};

enum class Mode : uint8_t {
  Mode0 = 0,
  Mode1 = 1,
  Mode2 = 2,
  Mode3 = 3,
};

enum class ChipSelect : uint8_t {
  CS0 = 0,
  CS1 = 1,
};

enum class ChipSelectPolicy : uint8_t {
  Hardware,
  Gpio,
  None,
};

struct DeviceConfig {
  Mode mode;
  uint16_t clockDivider;
  ChipSelect chipSelect;
  ChipSelectPolicy chipSelectPolicy;
};

class Device {
public:
  Device(Driver::BSP::BCM::SPI *spi, Driver::BSP::BCM::GPIO *gpio,
         const DeviceConfig &config, const ControlPins &pins);

  void begin() const;
  void end() const;
  void write(const uint8_t *data, size_t length) const;
  uint8_t transfer(uint8_t value) const;
  void setDataMode() const;
  void setCommandMode() const;
  void hardwareReset(unsigned int holdMillis, unsigned int settleMillis) const;

  const DeviceConfig &deviceConfig() const { return config; }
  const ControlPins &controlPins() const { return pins; }

private:
  Driver::BSP::BCM::SPI *spi;
  Driver::BSP::BCM::GPIO *gpio;
  DeviceConfig config;
  ControlPins pins;
};

} // namespace Driver::SPI
