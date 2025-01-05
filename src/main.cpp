#include "console.hpp"
#include "bsp/raspberrypi/qemu_console.hpp"
#include "std/printf.h"
#include "bsp/raspberrypi/memory.hpp"
#include "driver.hpp"
#include "bsp/device_driver/bcm/bcm2xxx_gpio.hpp"

extern "C" void putchar_(char c) {
    Console::console().printChar(c);
}

void postInitGpio() {
    Console::console().printLine("GPIO driver initialized");
}

void kernel_init() {
    Console::setConsole(new Console::QemuConsole());
    Console::console().printLine("Hello, %s!", "world");
    Driver::BSP::BCM::GPIO gpio = Driver::BSP::BCM::GPIO(Memory::Map::getMMIO().GPIO_START);
    Driver::driverManager().addDriver(Driver::DeviceDriverDescriptor(&gpio, postInitGpio));
    Driver::driverManager().printDrivers();
}

void kernel_main() {
    while (true)
    {
    }
}
