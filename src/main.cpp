#include "console.hpp"
#include "qemu_console.hpp"
#include "printf.h"
#include "memory.hpp"
#include "driver.hpp"
#include "device_driver/bcm/bcm2xxx_gpio.hpp"

extern "C" void putchar_(char c) {
    Console::console.printChar(c);
}

void postInitGpio() {
    Console::console.printLine("GPIO driver initialized");
}

void kernel_init() {
    Console::setConsole(Console::QemuConsole());
    Console::console.printLine("Hello, %s!", "world");
    Driver::GPIO gpio;
    Driver::driverManager.addDriver(Driver::DeviceDriverDescriptor(&gpio, postInitGpio));
    Driver::driverManager.printDrivers();
}

void kernel_main() {
    while (true)
    {
    }
}