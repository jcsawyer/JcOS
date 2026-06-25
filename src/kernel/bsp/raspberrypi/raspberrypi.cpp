#include "raspberrypi.hpp"
#include "../../driver/driver.hpp"
#include "exception/asynchronous.hpp"
#include "memory.hpp"
#include "memory/mmu.hpp"

namespace Driver {
namespace BSP {
namespace RaspberryPi {

Driver::BSP::BCM::InterruptController *RaspberryPi::interruptController =
    nullptr;
BCM::GPIO *RaspberryPi::gpio = nullptr;
BCM::SPI0 *RaspberryPi::spi0 = nullptr;
BCM::UART *RaspberryPi::uart = nullptr;
BCM::RNG *RaspberryPi::rng = nullptr;
BCM::UART::UartConsole *RaspberryPi::uartConsole = nullptr;
BCM::Timer *RaspberryPi::timer = nullptr;
Driver::Display::Display *RaspberryPi::display = nullptr;

Driver::BSP::BCM::InterruptController *RaspberryPi::getInterruptController() {
#if BOARD == bsp_rpi3
  if (interruptController == nullptr) {
    const size_t mappedLocalInterruptController = Memory::kernelMapMMIO(
        "BCM Local Interrupt Controller",
        Memory::MMIODescriptor(
            Memory::Map::getMMIO().LOCAL_INTERRUPT_CONTROLLER_START,
            Memory::Map::LOCAL_INTERRUPT_CONTROLLER_SIZE));
    const size_t mappedInterruptController = Memory::kernelMapMMIO(
        "BCM Interrupt Controller",
        Memory::MMIODescriptor(Memory::Map::getMMIO().START +
                                   Memory::Map::INTERRUPT_CONTROLLER_OFFSET,
                               Memory::Map::INTERRUPT_CONTROLLER_SIZE));
    static Driver::BSP::BCM::InterruptController interruptControllerInstance(
        mappedLocalInterruptController, mappedInterruptController);
    interruptController = &interruptControllerInstance;
  }
  return interruptController;
#else
#error "Unsupported board configuration"
#endif
}

BCM::GPIO *RaspberryPi::getGPIO() {
  if (gpio == nullptr) {
    const size_t mappedGPIO = Memory::kernelMapMMIO(
        "BCM GPIO", Memory::MMIODescriptor(Memory::Map::getMMIO().GPIO_START,
                                           Memory::Map::GPIO_SIZE));
    static BCM::GPIO gpioInstance(mappedGPIO);
    gpio = &gpioInstance;
  }
  return gpio;
}

BCM::SPI0 *RaspberryPi::getSPI0() {
  if (spi0 == nullptr) {
    const size_t mappedSPI0 = Memory::kernelMapMMIO(
        "BCM SPI0", Memory::MMIODescriptor(Memory::Map::getMMIO().START +
                                               Memory::Map::SPI0_OFFSET,
                                           Memory::Map::SPI0_SIZE));
    static BCM::SPI0 spi0Instance(mappedSPI0);
    spi0 = &spi0Instance;
  }
  return spi0;
}

BCM::UART *RaspberryPi::getUART() {
  if (uart == nullptr) {
    const size_t mappedUART = Memory::kernelMapMMIO(
        "BCM PL011 UART",
        Memory::MMIODescriptor(Memory::Map::getMMIO().PL011_UART_START,
                               Memory::Map::UART_SIZE));
    static BCM::UART uartInstance(mappedUART);
    uart = &uartInstance;
  }
  return uart;
}

BCM::RNG *RaspberryPi::getRNG() {
  if (rng == nullptr) {
    const size_t mappedRNG = Memory::kernelMapMMIO(
        "BCM RNG", Memory::MMIODescriptor(Memory::Map::getMMIO().RNG_START,
                                          Memory::Map::RNG_SIZE));
    static BCM::RNG rngInstance(mappedRNG);
    rng = &rngInstance;
  }
  return rng;
}

BCM::UART::UartConsole *RaspberryPi::getUartConsole() {
  if (uartConsole == nullptr) {
    static BCM::UART::UartConsole uartConsoleInstance(getUART());
    uartConsole = &uartConsoleInstance;
  }
  return uartConsole;
}

Driver::Display::Display *RaspberryPi::getDisplay() {
  if (display == nullptr) {
    static LCD::SpiTft displayInstance(getGPIO(), getSPI0());
    display = &displayInstance;
  }
  return display;
}

BCM::Timer *RaspberryPi::getTimer() {
  const size_t mappedTimer = Memory::kernelMapMMIO(
      "BCM Timer", Memory::MMIODescriptor(Memory::Map::getMMIO().TIMER_START,
                                          Memory::Map::TIMER_SIZE));
  static BCM::Timer timerInstance(mappedTimer);
  return &timerInstance;
}

void RaspberryPi::init() {
  // The timer driver registers its IRQ handler before the interrupt-controller
  // descriptor's post-init runs, so the controller must be initialized and
  // published before DriverManager::initDriversAndIrqs() enters its IRQ phase.
  getInterruptController()->init();
  Exceptions::Asynchronous::registerIrqManager(getInterruptController());

  driverManager().addDriver(DeviceDriverDescriptor(getGPIO(), &postInitGpio));
  driverManager().addDriver(DeviceDriverDescriptor(getUART(), &postInitUart));
  driverManager().addDriver(DeviceDriverDescriptor(getSPI0(), &postInitSPI0));
  driverManager().addDriver(
      DeviceDriverDescriptor(getDisplay(), &postInitDisplay));
  driverManager().addDriver(DeviceDriverDescriptor(getRNG(), &postInitRng));
  // driverManager().addDriver(DeviceDriverDescriptor(getTimer(),
  // &postInitTimer));
  driverManager().addDriver(DeviceDriverDescriptor(
      getInterruptController(), &postInitInterruptController));
}

void RaspberryPi::postInitUart() {
  Console::Console::SetInstance(getUartConsole());
}

void RaspberryPi::postInitGpio() { getGPIO()->mapPl011Uart(); }

void RaspberryPi::postInitSPI0() {}

void RaspberryPi::postInitRng() {}

void RaspberryPi::postInitDisplay() {}

void RaspberryPi::postInitTimer() { getTimer()->timerInit(); }

void RaspberryPi::postInitInterruptController() {}

} // namespace RaspberryPi
} // namespace BSP
} // namespace Driver
