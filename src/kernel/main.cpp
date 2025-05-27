#include "bsp/device_driver/lcd/hd44780u.hpp"
#include <bsp/bsp.hpp>
#include <console/console.hpp>
#include <driver/driver.hpp>
#include <exception.hpp>
#include <main.hpp>
#include <memory/mmu.hpp>
#include <print.hpp>
#include <time/duration.hpp>

extern "C" void putchar_(const char c) {
  Console::Console *console = Console::Console::GetInstance();
  console->printChar(c);
}

auto logo = R"""(
    __
   |  |    _____ _____
 __|  |___|     |   __|
|  |  |  _|  |  |__   |
|_____|___|_____|_____|
         Version 0.1.0


)""";

[[noreturn]] void kernel_main() {
  Console::Console *console = Console::Console::GetInstance();

  console->print(logo);
  info("%s version %s", "JcOS", "0.1.0");
  BSP::Board::PrintInfo();

  info("MMU online. Special regions:");
  Memory::virtMemLayout()->printLayout();

  const char *privilegeLevel;
  Exception::current_privilege_level(&privilegeLevel);
  info("Current privilege level: %s", privilegeLevel);

  info("Exception handling state:");
  Exception::print_state();

  info("Drivers loaded:");
  Driver::driverManager().printDrivers();

  Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
  info("Timer test, spinning for 1 second...");
  timeManager->spinFor(Time::Duration::from_secs(1));

  info("Echoing input now");
  while (true) {
    const char c = console->readChar();
    console->printChar(c);
  }
}

extern "C" void kernel_init() {
  const Memory::MemoryManagementUnit *mmu = Memory::MMU();
  Exception::handlingInit();
  mmu->enableMMUAndCaching();

  Time::TimeManager::GetInstance()->init();
  BSP::Board::init();
  Driver::driverManager().init();

  kernel_main();
}
