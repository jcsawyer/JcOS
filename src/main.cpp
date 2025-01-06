#include "bsp/device_driver/bcm/bcm2xxx_pl011_uart.hpp"
#include "bsp/raspberrypi/raspberrypi.hpp"
#include "console/console.hpp"
#include "std/printf.h"

extern "C" void _putchar(char c) {
  Console::Console *console = Console::Console::GetInstance();
  console->printChar(c);
}

const char *logo = R"""(
    __
   |  |    _____ _____
 __|  |___|     |   __|
|  |  |  _|  |  |__   |
|_____|___|_____|_____|
         Version 0.1.0


)""";

void kernel_main() {
  Console::Console *console = Console::Console::GetInstance();
  console->print(logo);
  console->printLine("%s version %s", "JcOS", "0.1.0");
  console->printLine("Booting on Raspeberry Pi 3B+");
  console->printLine("Drivers loaded:");
  Driver::driverManager().printDrivers();
  while (true) {
    asm volatile("nop");
  }
}

void kernel_init() {
  Driver::BSP::RaspberryPi::RaspberryPi::init();
  Driver::driverManager().init();

  kernel_main();
}
