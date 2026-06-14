#include "driver.hpp"

namespace Driver {
static auto _driverManager = DriverManager();

void DriverManager::initDriversAndIrqs() const {
  for (size_t i = 0; i < drivers.size(); ++i) {
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

  // 3. Register and enable IRQ handlers for all drivers
  for (size_t i = 0; i < drivers.size(); ++i) {
    DeviceDriverDescriptor descriptor = drivers[i];
    DeviceDriver *driver = descriptor.getDriver();

    if (driver && descriptor.getIrqNumber()) {
      driver->registerAndEnableIrqHandler(descriptor.getIrqNumber());
    }
  }
}

DriverManager &driverManager() { return _driverManager; }
} // namespace Driver
