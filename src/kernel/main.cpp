#include "main.hpp"
#include "_arch/aarch64/exception.hpp"
#include "_arch/aarch64/exception/asynchronous.hpp"
#include "_arch/aarch64/memory/mmu.hpp"
#include "_arch/time.hpp"
#include "bsp/raspberrypi.hpp"
#include "bsp/raspberrypi/raspberrypi.hpp"
#include "console/console.hpp"
#include <duration.hpp>
#include <exception.hpp>
#include <print.hpp>
#include <time.hpp>
#include "bsp/raspberrypi/memory/mmu.hpp"

#include "_arch/aarch64/memory/mmu.hpp"

extern "C" void putchar_(char c) {
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

[[noreturn]] void kernel_main() {
  Console::Console *console = Console::Console::GetInstance();

Memory::kernelTables()->populateTTEntries();
Memory::virtMemLayout()->printLayout();


  console->print(logo);
  info("%s version %s", "JcOS", "0.1.0");
  info("Booting on: %s", RaspberryPi::boardName());

  info("MMU online. Special regions:");
  Memory::virtMemLayout()->printLayout();

  const char *privilegeLevel;
  Exception::current_privilege_level(&privilegeLevel);
  info("Current privilege level: %s", privilegeLevel);

  info("Exception handling state:");
  Exception::print_state();

  info("Drivers loaded:");
  Driver::driverManager().printDrivers();
  info("Today's random number: %d",
       Driver::BSP::RaspberryPi::RaspberryPi::getRNG()->next(0, 100));

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


  Memory::MemoryManagementUnit* mmu = Memory::MMU();
  mmu->enableMMUAndCaching();

  Time::TimeManager::GetInstance()->init();
  Driver::BSP::RaspberryPi::RaspberryPi::init();
  Driver::driverManager().init();

  kernel_main();
}
