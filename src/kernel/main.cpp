#include "main.hpp"
#include "_arch/aarch64/exception.hpp"
#include "_arch/aarch64/exception/asynchronous.hpp"
#include "_arch/aarch64/memory/mmu.hpp"
#include "_arch/time.hpp"
#include "bsp/raspberrypi.hpp"
#include "bsp/raspberrypi/memory/mmu.hpp"
#include "bsp/raspberrypi/raspberrypi.hpp"
#include "console/console.hpp"
#include <duration.hpp>
#include <exception.hpp>
#include <print.hpp>
#include <time.hpp>

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

  console->print(logo);
  info("%s version %s", "JcOS", "0.1.0");
  info("Booting on: %s", RaspberryPi::boardName());

  info("MMU online. Special regions:");
  Memory::virtMemLayout()->printLayout();

  const char *privilegeLevel;
  Exception::CurrentEL::current_privilege_level(&privilegeLevel);
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

  // Get the value of the CurrentEL register
  const char *el_string;
  Exception::CurrentEL::current_privilege_level(&el_string);
  info("Current exception level: %s", el_string);

  // Get the value of VBAR_EL1
  uint64_t vbar_el1 = 0;
  asm volatile("mrs %0, vbar_el1" : "=r"(vbar_el1));

  info("Trying to read from address 8 GiB...");

  unsigned long long big_addr = 8ULL * 1024ULL * 1024ULL * 1024ULL;
  volatile unsigned long long *ptr =
      reinterpret_cast<volatile unsigned long long *>(big_addr);
  unsigned long long value = *ptr;

  info("************************************************");
  info("Whoa! We recovered from a synchronous exception!");
  info("************************************************");
  info("");

  info("Let's try again");
  big_addr = 9ULL * 1024ULL * 1024ULL * 1024ULL;
  ptr = reinterpret_cast<volatile unsigned long long *>(big_addr);
  value = *ptr;

  info("Echoing input now");
  while (true) {
    const char c = console->readChar();
    console->printChar(c);
  }
}

extern "C" void kernel_init() {
  Memory::MemoryManagementUnit *mmu = Memory::MMU();
  Exception::handlingInit();
  mmu->enableMMUAndCaching();

  Time::TimeManager::GetInstance()->init();
  Driver::BSP::RaspberryPi::RaspberryPi::init();
  Driver::driverManager().init();

  kernel_main();
}
