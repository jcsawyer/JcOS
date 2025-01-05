#pragma once

#include "../console/console.hpp"

namespace Driver {
    const int NUM_DRIVERS = 5;

    typedef void (*DeviceDriverPostInitCallback)();

    class DeviceDriver {
    public:
        DeviceDriver() = default;
        virtual char* compatible();
        virtual void init();
    };

    class DeviceDriverDescriptor {
    public:
        DeviceDriverDescriptor() = default;
        DeviceDriverDescriptor(DeviceDriverPostInitCallback callback)
                : driver(), postInitCallback(callback) {}

        DeviceDriverDescriptor(DeviceDriver* driver, DeviceDriverPostInitCallback callback)
            : driver(driver), postInitCallback(callback) {}

        DeviceDriver* getDriver() {
            return driver;
        }

        DeviceDriverPostInitCallback getPostInitCallback() const {
            return postInitCallback;
        }

    private:
        DeviceDriver* driver;
        DeviceDriverPostInitCallback postInitCallback;
    };

    class DriverManager {
    public:
        DriverManager() = default;
        void addDriver(const DeviceDriverDescriptor& descriptor) {
            if (nextIndex >= NUM_DRIVERS) {
                return;
            }
            drivers[nextIndex++] = descriptor;
        }

        void init() {
            for (int i = 0; i < nextIndex; ++i) {
                DeviceDriverDescriptor descriptor = drivers[i];
                DeviceDriver* driver = descriptor.getDriver();
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
                DeviceDriver* driver = drivers[i].getDriver();
                if (driver) {
                    const char* compatible = driver->compatible();
                    Console::console().printLine("Driver %d: %s", i + 1, compatible);
                }
            }
        }

    private:
        DeviceDriverDescriptor drivers[NUM_DRIVERS];
        int nextIndex;
    };

    DriverManager& driverManager();
}
