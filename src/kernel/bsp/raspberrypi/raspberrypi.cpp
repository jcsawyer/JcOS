#include "raspberrypi.hpp"
#include "../../driver/driver.hpp"
#include "exception/asynchronous.hpp"
#include "memory.hpp"

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
    static Driver::BSP::BCM::InterruptController interruptControllerInstance(
        0x3F000000 + Memory::Map::INTERRUPT_CONTROLLER_OFFSET);
    interruptController = &interruptControllerInstance;
  }
  return interruptController;
#else
#error "Unsupported board configuration"
#endif
}

BCM::GPIO *RaspberryPi::getGPIO() {
  if (gpio == nullptr) {
    static BCM::GPIO gpioInstance(Memory::Map::getMMIO().GPIO_START);
    gpio = &gpioInstance;
  }
  return gpio;
}

BCM::UART *RaspberryPi::getUART() {
  if (uart == nullptr) {
    static BCM::UART uartInstance(Memory::Map::getMMIO().PL011_UART_START);
    uart = &uartInstance;
  }
  return uart;
}

BCM::RNG *RaspberryPi::getRNG() {
  if (rng == nullptr) {
    static BCM::RNG rngInstance(Memory::Map::getMMIO().RNG_START);
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
    static LCD::HD44780U lcdInstance(Memory::Map::getMMIO().GPIO_START);
    lcd = &lcdInstance;
  }
  return lcd;
}

BCM::Timer *RaspberryPi::getTimer() {
  static BCM::Timer timerInstance(Memory::Map::getMMIO().TIMER_START);
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