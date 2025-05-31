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

extern void timerInit();

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

void inputEchoTask() {
  info("Echoing input now");
  Console::Console *console = Console::Console::GetInstance();
  Driver::BSP::LCD::HD44780U *lcd =
      Driver::BSP::RaspberryPi::RaspberryPi::getLCD();
  while (true) {
    const char c = console->readChar();
    console->printChar(c);

    const unsigned char printChar[2] = {c, '\0'};
    lcd->writeString(reinterpret_cast<const char *>(printChar));
    taskManager.schedule(); // Yield to the scheduler
  }
}

void task1() {
  Driver::BSP::LCD::HD44780U *lcd =
      Driver::BSP::RaspberryPi::RaspberryPi::getLCD();
  while (1) {
    Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
    lcd->setCursor(0, 0);
    lcd->writeString("Task 1");
    timeManager->spinFor(Time::Duration::from_secs(0.5));
    taskManager.schedule(); // Yield to the scheduler
  }
}

void task2() {
  Driver::BSP::LCD::HD44780U *lcd =
      Driver::BSP::RaspberryPi::RaspberryPi::getLCD();
  while (1) {
    Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
    lcd->setCursor(0, 0);
    lcd->writeString("Task 2");
    timeManager->spinFor(Time::Duration::from_secs(0.5));
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
  taskManager.addTask("Terminal", inputEchoTask);
  taskManager.addTask("Task 1", task1);
  taskManager.addTask("Task 2", task2);

  timerInit();

  taskManager.currentTask = 0;
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
