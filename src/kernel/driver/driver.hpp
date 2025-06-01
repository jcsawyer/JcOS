#pragma once

#include "../print.hpp"
#include <bsp/exception/asynchronous.hpp>
#include <memory.h>
#include <stddef.h>

namespace Driver {
const int NUM_DRIVERS = 6;

typedef void (*DeviceDriverPostInitCallback)();

class DeviceDriver {
public:
  virtual const char *compatible() = 0;
  virtual void init() = 0;
  virtual ~DeviceDriver() = default;
  virtual void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) = 0;
};

class DeviceDriverDescriptor {
public:
  DeviceDriverDescriptor()
      : driver(), postInitCallback(nullptr), irqNumber(nullptr) {}
  DeviceDriverDescriptor(DeviceDriverPostInitCallback callback)
      : driver(), postInitCallback(callback), irqNumber(nullptr) {}

  DeviceDriverDescriptor(DeviceDriver *driver,
                         DeviceDriverPostInitCallback callback)
      : driver(driver), postInitCallback(callback), irqNumber(nullptr) {}

  DeviceDriverDescriptor(DeviceDriver *driver,
                         DeviceDriverPostInitCallback callback,
                         ::BSP::Exception::Asynchronous::IRQNumber *irqNumber)
      : driver(driver), postInitCallback(callback), irqNumber(irqNumber) {}

  DeviceDriver *getDriver() { return driver; }

  ::BSP::Exception::Asynchronous::IRQNumber *getIrqNumber() {
    return irqNumber;
  }

  DeviceDriverPostInitCallback getPostInitCallback() const {
    return postInitCallback;
  }

private:
  DeviceDriver *driver;
  DeviceDriverPostInitCallback postInitCallback;
  ::BSP::Exception::Asynchronous::IRQNumber *irqNumber;
};

class DriverManager {
public:
  void addDriver(const DeviceDriverDescriptor &descriptor) {
    if (nextIndex >= NUM_DRIVERS) {
      return;
    }
    drivers[nextIndex++] = descriptor;
  }

  void initDriversAndIrqs() const;

  void printDrivers() {
    for (int i = 0; i < nextIndex; ++i) {
      DeviceDriver *driver = drivers[i].getDriver();
      if (driver) {
        const char *compatible = driver->compatible();
        info("      %d: %s", i + 1, compatible);
      }
    }
  }

private:
  DeviceDriverDescriptor drivers[NUM_DRIVERS];
  int nextIndex;
};

DriverManager &driverManager();
} // namespace Driver
