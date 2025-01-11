#include "driver.hpp"

namespace Driver {
static auto _driverManager = DriverManager();

DriverManager &driverManager() { return _driverManager; }
} // namespace Driver
