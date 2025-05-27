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

  info("Requsting kernel binary...");
  console->flush();

  console->clearRx();

  for (int i = 0; i < 3; ++i) {
    console->printChar(3);
  }

  // Read 4 bytes (little-endian) as binary size
  uint32_t size = static_cast<uint8_t>(console->readChar());
  size |= static_cast<uint8_t>(console->readChar()) << 8;
  size |= static_cast<uint8_t>(console->readChar()) << 16;
  size |= static_cast<uint8_t>(console->readChar()) << 24;

  // Acknowledge
  console->printChar('O');
  console->printChar('K');

  // Get the address where the kernel should be loaded
  uint8_t *kernel_addr = reinterpret_cast<uint8_t *>(0x80000);

  // Read binary into memory
  for (uint32_t i = 0; i < size; ++i) {
    uint8_t byte = static_cast<uint8_t>(console->readChar());
    *(volatile uint8_t *)(kernel_addr + i) = byte;
  }

  info("Loaded! Executing the payload now\n");
  console->flush();

  // Cast memory to function pointer and jump
  using KernelEntry = void (*)(); // assuming kernel never returns
  KernelEntry kernel = reinterpret_cast<KernelEntry>(kernel_addr);
  kernel();

  // Should never return
  while (true) {
  }
}

extern "C" void kernel_init() {
  // const Memory::MemoryManagementUnit *mmu = Memory::MMU();
  // Exception::handlingInit();
  // mmu->enableMMUAndCaching();

  Time::TimeManager::GetInstance()->init();
  BSP::Board::init();
  Driver::driverManager().init();

  kernel_main();
}
