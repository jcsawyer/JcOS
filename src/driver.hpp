#pragma once

#include "console.hpp"

namespace Driver {
    const int NUM_DRIVERS = 5;

    using DeviceDriverPostInitCallback = void (*)();

    // DeviceDriver class
    class DeviceDriver {
    public:
        virtual char* compatible();
        virtual void init();
    };

    // DeviceDriverDescriptor class
    class DeviceDriverDescriptor {
    public:
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

    // DriverManager class
    class DriverManager {
    public:
        void addDriver(const DeviceDriverDescriptor& descriptor) {
            if (nextIndex >= NUM_DRIVERS) {
                return; // Array is full
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
        int nextIndex = 0;
    };

    DriverManager& driverManager();
}
