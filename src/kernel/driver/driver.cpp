#include "driver.hpp"

namespace Driver {
static auto _driverManager = DriverManager();

void DriverManager::init() const {
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

DriverManager &driverManager() { return _driverManager; }
} // namespace Driver
