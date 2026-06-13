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
BCM::UART *RaspberryPi::uart = nullptr;
BCM::RNG *RaspberryPi::rng = nullptr;
BCM::UART::UartConsole *RaspberryPi::uartConsole = nullptr;
BCM::Timer *RaspberryPi::timer = nullptr;
LCD::HD44780U *RaspberryPi::lcd = nullptr;

Driver::BSP::BCM::InterruptController *RaspberryPi::getInterruptController() {
#if BOARD == bsp_rpi3
  if (interruptController == nullptr) {
    const size_t mappedInterruptController = Memory::kernelMapMMIO(
        "BCM Interrupt Controller",
        Memory::MMIODescriptor(Memory::Map::getMMIO().START +
                                   Memory::Map::INTERRUPT_CONTROLLER_OFFSET,
                               Memory::Map::INTERRUPT_CONTROLLER_SIZE));
    static Driver::BSP::BCM::InterruptController interruptControllerInstance(
        mappedInterruptController);
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

LCD::HD44780U *RaspberryPi::getLCD() {
  if (lcd == nullptr) {
    const size_t mappedGPIO = Memory::kernelMapMMIO(
        "HD44780U GPIO",
        Memory::MMIODescriptor(Memory::Map::getMMIO().GPIO_START,
                               Memory::Map::GPIO_SIZE));
    static LCD::HD44780U lcdInstance(mappedGPIO);
    lcd = &lcdInstance;
  }
  return lcd;
}

BCM::Timer *RaspberryPi::getTimer() {
  const size_t mappedTimer = Memory::kernelMapMMIO(
      "BCM Timer", Memory::MMIODescriptor(Memory::Map::getMMIO().TIMER_START,
                                          Memory::Map::TIMER_SIZE));
  static BCM::Timer timerInstance(mappedTimer);
  return &timerInstance;
}

void RaspberryPi::init() {
  driverManager().addDriver(DeviceDriverDescriptor(getGPIO(), &postInitGpio));
  driverManager().addDriver(DeviceDriverDescriptor(
      getUART(), &postInitUart, Driver::BSP::RaspberryPi::PL011_UART()));
  driverManager().addDriver(DeviceDriverDescriptor(getLCD(), &postInitLCD));
  driverManager().addDriver(DeviceDriverDescriptor(getRNG(), &postInitRng));
  // driverManager().addDriver(DeviceDriverDescriptor(getTimer(),
  // &postInitTimer));
  driverManager().addDriver(DeviceDriverDescriptor(
      getInterruptController(), &postInitInterruptController));
}

void RaspberryPi::postInitUart() {
  LCD::HD44780U *lcd = getLCD();
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->writeString("Initializing UART...");
  Console::Console::SetInstance(getUartConsole());
}

void RaspberryPi::postInitGpio() { getGPIO()->mapPl011Uart(); }

void RaspberryPi::postInitRng() {
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->writeString("Initializing RNG...");
}

void RaspberryPi::postInitLCD() {
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->writeString("Initializing LCD...");
}

void RaspberryPi::postInitTimer() {

  getTimer()->timerInit();

  LCD::HD44780U *lcd = getLCD();
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->writeString("Initializing Timer...");
}

void RaspberryPi::postInitInterruptController() {
  LCD::HD44780U *lcd = getLCD();
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->writeString("Initializing Interrupts...");
  Exceptions::Asynchronous::registerIrqManager(getInterruptController());
}

} // namespace RaspberryPi
} // namespace BSP
} // namespace Driver
