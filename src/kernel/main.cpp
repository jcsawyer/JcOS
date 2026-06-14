#include "bsp/raspberrypi/raspberrypi.hpp"
#include <bsp/bsp.hpp>
#include <bsp/raspberrypi/memory/mmu.hpp>
#include <common.hpp>
#include <console/console.hpp>
#include <driver/driver.hpp>
#include <exception.hpp>
#include <exceptions/asynchronous.hpp>
#include <main.hpp>
#include <memory/heap.hpp>
#include <memory/mmu.hpp>
#include <display/display.hpp>
#include <task.hpp>
#include <time/duration.hpp>

#include <bsp/device_driver/bcm/bcm2xxx_interrupt_controller/peripheral_ic.hpp>

#include <state.hpp>
#include <stdio/printf.h>
#include <synchronization.hpp>
#include <syscall.hpp>

namespace {

constexpr unsigned int kLcdColumns = 16;

void writePaddedLcdLine(Driver::BSP::LCD::HD44780U &lcd, unsigned char row,
                        const char *text) {
  char line[kLcdColumns + 1];
  unsigned int i = 0;

  for (; i < kLcdColumns && text[i] != '\0'; ++i) {
    line[i] = text[i];
  }

  for (; i < kLcdColumns; ++i) {
    line[i] = ' ';
  }

  line[kLcdColumns] = '\0';
  lcd.setCursor(row, 0);
  lcd.writeString("%s", line);
}

void formatHeapSize(char *buffer, size_t bufferSize, size_t bytes) {
  if (bytes >= MiB) {
    snprintf_(buffer, bufferSize, "%luM", div_ceil(bytes, MiB));
    return;
  }

  if (bytes >= KiB) {
    snprintf_(buffer, bufferSize, "%luK", div_ceil(bytes, KiB));
    return;
  }

  snprintf_(buffer, bufferSize, "%luB", bytes);
}

void renderHeapUsageOnLcd() {
  auto &heap = Memory::kernel_heap_allocator();
  if (!heap.isInitialized()) {
    return;
  }

  Driver::BSP::LCD::HD44780U *lcd =
      Driver::BSP::RaspberryPi::RaspberryPi::getLCD();

  char used[6];
  char total[6];
  char free[6];
  char line[kLcdColumns + 1];

  formatHeapSize(used, sizeof(used), heap.usedBytesCount());
  formatHeapSize(total, sizeof(total), heap.totalBytes());
  formatHeapSize(free, sizeof(free), heap.freeBytesCount());

  snprintf_(line, sizeof(line), "Heap %s/%s", used, total);
  writePaddedLcdLine(*lcd, 0, line);

  snprintf_(line, sizeof(line), "Free %s", free);
  writePaddedLcdLine(*lcd, 1, line);
}

} // namespace

extern void timerInit();

extern "C" void putchar_(const char c) {
  Console::Console *console = Console::Console::GetInstance();
  console->printChar(c);
}

auto logo = R"""(
    __
   |  |    _____ _____
   |  |___|     |   __|
 __|  |  _|  |  |__   |
|_____|___|_____|_____|
         Version 0.1.0


)""";

void utoa(unsigned int value, char *buffer) {
  int i = 0;
  if (value == 0) {
    buffer[i++] = '0';
  } else {
    while (value > 0) {
      buffer[i++] = '0' + (value % 10);
      value /= 10;
    }
  }
  buffer[i] = '\0';
}

[[noreturn]] __attribute__((noinline)) void verifyBacktraceFromKernelMain() {
  // Real-device backtrace smoke test:
  // 1. Uncomment the call in kernel_main().
  // 2. Deploy to the board and capture the serial log.
  // 3. Confirm the panic output includes `Backtrace:` and resolves this
  //    function plus kernel_main() in the trace.
  *reinterpret_cast<volatile uint64_t *>(0x1) = 0;
  while (true) {
  }
}

[[noreturn]] __attribute__((noinline)) void verifyBacktraceFromTaskPanic() {
  // Real-device backtrace smoke test for task stacks:
  // 1. Uncomment the call in task1().
  // 2. Deploy to the board and capture the serial log.
  // 3. Confirm the panic output includes `Backtrace:` and resolves this
  //    function plus task1() in the trace.
  panic("Intentional task panic for backtrace verification");
}

[[noreturn]] __attribute__((noinline)) void verifyBacktraceFromTaskError() {
  // Real-device backtrace smoke test for task-stack exceptions:
  // 1. Uncomment the call in task1().
  // 2. Deploy to the board and capture the serial log.
  // 3. Confirm the exception dump and panic output include `Backtrace:` and
  //    resolve this function plus task1() in the trace.
  *reinterpret_cast<volatile uint64_t *>(0x1) = 0;
  while (true) {
  }
}

void task1() {
  while (1) {
    info("Task 1 running...");
    Syscall::write("Hello from syscall::write!!\n");
    // Uncomment to verify backtrace output on a real device from a task stack.
    // verifyBacktraceFromTaskPanic();
    // Uncomment to verify task-stack backtrace output for a synchronous
    // exception on a real device.
    // verifyBacktraceFromTaskError();
    // Syscall::exit(0);
    Time::TimeManager *timeManager = Time::TimeManager::GetInstance();

    for (int i = 0; i < 10; i++) {
      renderHeapUsageOnLcd();
      taskManager.schedule(); // Yield to the scheduler
      timeManager->spinFor(Time::Duration::from_millis(100));
    }
  }
}

void task2() {
  while (1) {
    info("Running Task 2");
    Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
    for (int i = 0; i < 10; i++) {
      renderHeapUsageOnLcd();
      taskManager.schedule(); // Yield to the scheduler
      timeManager->spinFor(Time::Duration::from_millis(100));
    }
  }
}

[[noreturn]] void kernel_main() {
  Console::Console *console = Console::Console::GetInstance();

  console->print(logo);
  info("%s version %s", "JcOS", "0.1.0");
  BSP::Board::PrintInfo();

  info("MMU online:");
  Memory::kernelPrintMappings();

  const char *privilegeLevel;
  Exception::current_privilege_level(&privilegeLevel);
  info("Current privilege level: %s", privilegeLevel);

  info("Exception handling state:");
  Exception::print_state();

  info("Drivers loaded:");
  Driver::driverManager().printDrivers();

  info("Registered IRQ handlers:");
  Exceptions::Asynchronous::IRQManager *irqManager =
      Exceptions::Asynchronous::irq_manager();
  irqManager->printHandler();

  int *heapSmoke = new int(42);
  info("Kernel heap smoke test value: %d", *heapSmoke);
  delete heapSmoke;

  info("Kernel heap:");
  Memory::kernel_heap_allocator().printUsage();

  Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
  info("Timer test, spinning for 1 second...");
  timeManager->spinFor(Time::Duration::from_secs(1));

  // Uncomment to verify backtrace output on a real device from the boot/kernel
  // stack. This intentionally triggers a synchronous abort.
  // verifyBacktraceFromKernelMain();

  Driver::BSP::LCD::HD44780U *lcd =
      Driver::BSP::RaspberryPi::RaspberryPi::getLCD();
  lcd->clear();
  renderHeapUsageOnLcd();

  Driver::BSP::Display::SPITFTDisplay *tft =
      Driver::BSP::RaspberryPi::RaspberryPi::getTftDisplay();
  tft->fillScreen(0x001F);
  tft->fillRect(16, 16, 48, 48, 0xF800);
  tft->fillRect(72, 16, 48, 48, 0x07E0);
  tft->fillRect(128, 16, 48, 48, 0xFFFF);

  info("Task system initializing...");
  info("Task system initialized, starting task scheduler...");
  taskManager.init();
  taskManager.addTask("Task 1", task1);
  taskManager.addTask("Task 2", task2);

  timerInit();

  taskManager.currentTask = 0;

  while (true) {
    taskManager.schedule();
  }
  CPU::waitForever();
}

extern "C" void kernel_init() {
  Exception::handlingInit();
  Time::TimeManager::GetInstance()->init();
  Memory::kernel_init_heap_allocator();
  info("Kernel heap online");

  // Initialize the BSP driver subsystem
  BSP::Board::init();

  // Initialize all device drivers
  Driver::driverManager().initDriversAndIrqs();

  // Unamask interrupts on boot CPU core.
  Exception::Asynchronous::localIrqUnmask();

  // TODO figure out why not working on real hardware...
  // State::state_manager().transition_to_single_core_main();

  kernel_main();
}
