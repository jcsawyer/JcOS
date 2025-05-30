#include "bsp/raspberrypi/raspberrypi.hpp"
#include <bsp/bsp.hpp>
#include <console/console.hpp>
#include <driver/driver.hpp>
#include <exception.hpp>
#include <main.hpp>
#include <memory/mmu.hpp>
#include <print.hpp>
#include <task.hpp>
#include <time/duration.hpp>

extern void timer_init();

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

__attribute__((used, noinline)) void task1() {
  while (1) {
    info("Task 1");
    Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
    for (volatile int i = 0; i < 100000; ++i) {
    }
    taskManager.schedule(); // Yield to the scheduler
  }
}

__attribute__((used, noinline)) void task2() {
  while (1) {
    info("Task 2");
    Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
    for (volatile int i = 0; i < 100000; ++i)
      ;
    taskManager.schedule(); // Yield to the scheduler
  }
}

[[noreturn]] void kernel_main() {
  Console::Console *console = Console::Console::GetInstance();

  // console->print(logo);
  // info("%s version %s", "JcOS", "0.1.0");
  // BSP::Board::PrintInfo();

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

  Driver::BSP::LCD::HD44780U *lcd =
      Driver::BSP::RaspberryPi::RaspberryPi::getLCD();
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->writeString("JcOS v0.1.0");
  lcd->setCursor(1, 0);
  lcd->writeString(">");

  info("Task system initializing...");
  taskManager.init();
  info("Task system initialized, starting task scheduler...");
  taskManager.addTask(task1);
  taskManager.addTask(task2);

  timer_init();

  taskManager.currentTask = 0;

  info("Echoing input now");
  while (true) {
    const char c = console->readChar();
    console->printChar(c);

    const unsigned char printChar[2] = {c, '\0'};
    lcd->writeString(reinterpret_cast<const char *>(printChar));
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
