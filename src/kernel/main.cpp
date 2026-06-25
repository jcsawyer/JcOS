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

#include <bsp/device_driver/bcm/bcm2xxx_interrupt_controller/peripheral_ic.hpp>

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

void timeoutPeriodic1s() { info("Periodic 1 sec"); }

void timeoutOnce3s() { info("Once 3"); }

void timeoutOnce5s() { info("Once 5"); }

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

    taskManager.schedule(); // Yield to the scheduler
    timeManager->spinFor(Time::Duration::from_millis(100));
  }
}

void task2() {
  while (1) {
    info("Running Task 2");
    Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
    taskManager.schedule(); // Yield to the scheduler
    timeManager->spinFor(Time::Duration::from_millis(100));
  }
}

namespace {
constexpr uint16_t logoForeground = 0xFFFF;
constexpr uint16_t logoBackground = 0x0000;

void renderAsciiLogo(Driver::Display::Display *display, const char *asciiLogo) {
  if (display == nullptr || !display->isReady() || asciiLogo == nullptr) {
    return;
  }

  const char *lineStarts[8] = {};
  unsigned int lineLengths[8] = {};
  unsigned int lineCount = 0;
  unsigned int maxColumns = 0;

  const char *cursor = asciiLogo;
  while (*cursor != '\0' && lineCount < 8) {
    while (*cursor == '\n') {
      ++cursor;
    }

    if (*cursor == '\0') {
      break;
    }

    const char *lineStart = cursor;
    unsigned int lineLength = 0;
    while (*cursor != '\0' && *cursor != '\n') {
      ++cursor;
      ++lineLength;
    }

    if (lineLength == 0) {
      break;
    }

    lineStarts[lineCount] = lineStart;
    lineLengths[lineCount] = lineLength;
    if (lineLength > maxColumns) {
      maxColumns = lineLength;
    }
    ++lineCount;

    if (*cursor == '\n') {
      ++cursor;
    }

    if (lineCount == 5) {
      break;
    }
  }

  const unsigned int scale = display->width() >= 400 ? 2 : 1;
  const unsigned int glyphWidth = 6 * scale;
  const unsigned int glyphHeight = 8 * scale;
  const unsigned int artWidth = maxColumns * glyphWidth;
  const unsigned int artHeight = lineCount * glyphHeight;
  const unsigned int originX =
      display->width() > artWidth ? (display->width() - artWidth) / 2 : 0;
  const unsigned int originY = 32;

  for (unsigned int row = 0; row < lineCount; row++) {
    char lineBuffer[64];
    unsigned int length =
        lineLengths[row] < (sizeof(lineBuffer) - 1)
            ? lineLengths[row]
            : static_cast<unsigned int>(sizeof(lineBuffer) - 1);
    for (unsigned int col = 0; col < length; col++) {
      lineBuffer[col] = lineStarts[row][col];
    }
    lineBuffer[length] = '\0';

    display->drawString(originX, originY + row * glyphHeight, lineBuffer,
                        logoForeground, logoBackground, scale);
  }

  display->drawString(originX, originY + artHeight + 20, "Version 0.1.0",
                      logoForeground, logoBackground, scale);
}
} // namespace

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

  Driver::Display::Display *display =
      Driver::BSP::RaspberryPi::RaspberryPi::getDisplay();
  if (display->isReady()) {
    display->clear(logoBackground);
    renderAsciiLogo(display, logo);
  } else {
    warn("Display unavailable; skipping TFT logo render");
  }

  timeManager->setTimeoutOnce(Time::Duration::from_secs(5), timeoutOnce5s);
  timeManager->setTimeoutOnce(Time::Duration::from_secs(3), timeoutOnce3s);
  timeManager->setTimeoutPeriodic(Time::Duration::from_secs(1),
                                  timeoutPeriodic1s);

  info("Echoing input now");
  CPU::waitForever();
}

extern "C" void kernel_init() {
  Exception::handlingInit();
  Time::TimeManager::GetInstance()->earlyInit();
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
