// #include "console/console.hpp"

// extern "C" void _putchar(char c) { Console::console().printChar(c); }

// void kernel_main() {
//   while (true)
//     ;
// }

// void kernel_init() { kernel_main(); }

#include "bsp/raspberrypi/memory.hpp"
#include "bsp/raspberrypi/raspberrypi_driver.hpp"
#include "console/console.hpp"
#include "driver/driver.hpp"
#include "std/printf.h"

extern "C" void _putchar(char c) { Console::console().printChar(c); }

void postInitGpio() { Console::console().printLine("GPIO driver initialized"); }

void kernel_init() {
  Driver::BSP::RaspberryPi::init();
  Driver::driverManager().init();
}

void kernel_main() {
  Driver::driverManager().printDrivers();
  while (true)
    ;
}
