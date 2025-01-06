#pragma once

#include "../print.hpp"
#include "../std/stddef.h"

namespace Driver {
const int NUM_DRIVERS = 5;

typedef void (*DeviceDriverPostInitCallback)();

class DeviceDriver {
public:
  virtual const char *compatible() = 0;
  virtual void init() = 0;
  virtual ~DeviceDriver() = default;
};

class DeviceDriverDescriptor {
public:
  DeviceDriverDescriptor() : driver(), postInitCallback(nullptr) {}
  DeviceDriverDescriptor(DeviceDriverPostInitCallback callback)
      : driver(), postInitCallback(callback) {}

  DeviceDriverDescriptor(DeviceDriver *driver,
                         DeviceDriverPostInitCallback callback)
      : driver(driver), postInitCallback(callback) {}

  DeviceDriver *getDriver() { return driver; }

  DeviceDriverPostInitCallback getPostInitCallback() const {
    return postInitCallback;
  }

private:
  DeviceDriver *driver;
  DeviceDriverPostInitCallback postInitCallback;
};

class DriverManager {
public:
  void addDriver(const DeviceDriverDescriptor &descriptor) {
    if (nextIndex >= NUM_DRIVERS) {
      return;
    }
    drivers[nextIndex++] = descriptor;
  }

  void init() {
    for (int i = 0; i < nextIndex; ++i) {
      DeviceDriverDescriptor descriptor = drivers[i];
      DeviceDriver *driver = descriptor.getDriver();
      if (driver) {
        driver->init();
      }

      DeviceDriverPostInitCallback callback = descriptor.getPostInitCallback();
      if (callback) {
        callback();
      }
    }
  }

  void printDrivers() {
    for (int i = 0; i < nextIndex; ++i) {
      DeviceDriver *driver = drivers[i].getDriver();
      if (driver) {
        const char *compatible = driver->compatible();
        info("\t%d: %s", i + 1, compatible);
      }
    }
  }

private:
  DeviceDriverDescriptor drivers[NUM_DRIVERS];
  int nextIndex;
};

DriverManager &driverManager();
} // namespace Driver
