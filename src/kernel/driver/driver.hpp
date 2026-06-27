#pragma once

#include "../print.hpp"
#include <bsp/exception/asynchronous.hpp>
#include <container/vector.hpp>
#include <memory.h>
#include <stddef.h>

namespace Driver {
const int NUM_DRIVERS = 12;

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
    if (drivers.size() >= NUM_DRIVERS) {
      warn("DriverManager full; dropping driver registration");
      return;
    }
    drivers.pushBack(descriptor);
  }

  void initDriversAndIrqs() const;

  void printDrivers() {
    for (size_t i = 0; i < drivers.size(); ++i) {
      DeviceDriver *driver = drivers[i].getDriver();
      if (driver) {
        const char *compatible = driver->compatible();
        info("      %lu: %s", i + 1, compatible);
      }
    }
  }

private:
  Container::Vector<DeviceDriverDescriptor> drivers;
};

DriverManager &driverManager();
} // namespace Driver
