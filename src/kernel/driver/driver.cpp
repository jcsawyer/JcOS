#include "driver.hpp"

namespace Driver {
static DriverManager _driverManager = DriverManager();

DriverManager &driverManager() { return _driverManager; }
} // namespace Driver
