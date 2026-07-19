#include "arch/aarch64/cpu/dtb.hpp"
#include "bsp/raspberrypi/raspberrypi.hpp"
#include <bsp/bsp.hpp>
#include <bsp/raspberrypi/memory/mmu.hpp>
#include <console/console.hpp>
#include <driver/driver.hpp>
#include <exception.hpp>
#include <exceptions/asynchronous.hpp>
#include <main.hpp>
#include <memory/heap.hpp>
#include <memory/mmu.hpp>
#include <task.hpp>
#include <time/duration.hpp>
#include <ui.hpp>

#include <bsp/device_driver/bcm/bcm2xxx_interrupt_controller/peripheral_ic.hpp>
#include <bsp/device_driver/lcd/touch_controller.hpp>

#include <state.hpp>
#include <synchronization.hpp>
#include <syscall.hpp>

namespace {} // namespace

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

    Tasks::yield();
    timeManager->spinFor(Time::Duration::from_millis(100));
  }
}

void task2() {
  while (1) {
    info("Running Task 2");
    Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
    Tasks::yield();
    timeManager->spinFor(Time::Duration::from_millis(100));
  }
}

void idleTask() {
  while (1) {
    Tasks::yield();
  }
}

void memoryUiDemoTask() {
  Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
  constexpr size_t allocationSizes[] = {
      64 * KiB,
      192 * KiB,
      384 * KiB,
      128 * KiB,
  };
  constexpr size_t allocationCount =
      sizeof(allocationSizes) / sizeof(allocationSizes[0]);
  const Time::Duration holdAllocated = Time::Duration::from_millis(700);
  const Time::Duration holdFreed = Time::Duration::from_millis(400);
  size_t allocationIndex = 0;

  while (1) {
    const size_t allocationSize = allocationSizes[allocationIndex];
    allocationIndex = (allocationIndex + 1) % allocationCount;

    char *buffer = new char[allocationSize];
    if (allocationSize > 0) {
      buffer[0] = static_cast<char>(allocationIndex);
      buffer[allocationSize - 1] = static_cast<char>(allocationIndex);
    }

    Time::Duration releaseAt = timeManager->uptime() + holdAllocated;
    while (timeManager->uptime() < releaseAt) {
      Tasks::yield();
    }

    delete[] buffer;

    Time::Duration nextAllocationAt = timeManager->uptime() + holdFreed;
    while (timeManager->uptime() < nextAllocationAt) {
      Tasks::yield();
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

  taskManager.init();
  UI::Runtime *uiRuntime = UI::Runtime::GetInstance();
  Driver::Display::Display *display =
      Driver::BSP::RaspberryPi::RaspberryPi::getDisplay();
  uiRuntime->init(display);
  static UI::TouchInputSource touchInputSource(
      Driver::BSP::RaspberryPi::RaspberryPi::getTouchPanel(), display);
  uiRuntime->registerInputSource(&touchInputSource);
  uiRuntime->start();
  taskManager.addTask("memory-ui-demo", memoryUiDemoTask, 2);
  taskManager.addTask("idle", idleTask, 1, TaskState::Idle);

  info("Starting cooperative scheduler");
  taskManager.schedule();
  CPU::waitForever();
}

extern "C" void kernel_init() {
  Exception::handlingInit();
  Time::TimeManager::GetInstance()->earlyInit();
  if (CPU::Boot::DTB::cpuCoreCount() > 0) {
    info("Device tree reports %u CPU core(s)",
         static_cast<unsigned int>(CPU::Boot::DTB::cpuCoreCount()));
  } else {
    info("Device tree CPU core count unavailable");
  }
  Memory::kernel_init_heap_allocator();
  info("Kernel heap online");
  Time::registerTimerDriver();

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
