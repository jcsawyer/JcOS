#include "driver.hpp"

namespace Driver {
static auto _driverManager = DriverManager();

void DriverManager::initDriversAndIrqs() const {
  for (int i = 0; i < nextIndex; ++i) {
    DeviceDriverDescriptor descriptor = drivers[i];
    DeviceDriver *driver = descriptor.getDriver();

    // 1. Initialize the driver
    if (driver) {
      driver->init();
    }

    // 2. Call corresponding post-init callback if it exists
    DeviceDriverPostInitCallback callback = descriptor.getPostInitCallback();
    if (callback) {
      callback();
    }
  }

  for (int i = 0; i < nextIndex; ++i) {
    DeviceDriverDescriptor descriptor = drivers[i];
    DeviceDriver *driver = descriptor.getDriver();

    if (driver) {
      driver->registerAndEnableIrqHandler();
    }
  }
}

DriverManager &driverManager() { return _driverManager; }
} // namespace Driver
