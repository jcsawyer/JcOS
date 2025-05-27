#include "raspberrypi.hpp"
#include "../../driver/driver.hpp"
#include "memory.hpp"

namespace Driver {
namespace BSP {
namespace RaspberryPi {

BCM::GPIO *RaspberryPi::gpio = nullptr;
BCM::UART *RaspberryPi::uart = nullptr;
BCM::RNG *RaspberryPi::rng = nullptr;
BCM::UART::UartConsole *RaspberryPi::uartConsole = nullptr;
LCD::HD44780U *RaspberryPi::lcd = nullptr;

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

void RaspberryPi::init() {
  driverManager().addDriver(DeviceDriverDescriptor(getGPIO(), &postInitGpio));
  driverManager().addDriver(DeviceDriverDescriptor(getUART(), &postInitUart));
  driverManager().addDriver(DeviceDriverDescriptor(getLCD(), &postInitLCD));
  driverManager().addDriver(DeviceDriverDescriptor(getRNG(), &postInitRng));
}

void RaspberryPi::postInitUart() {
  Console::Console::SetInstance(getUartConsole());
}

void RaspberryPi::postInitGpio() { getGPIO()->mapPl011Uart(); }

void RaspberryPi::postInitRng() {}

void RaspberryPi::postInitLCD() {
  getLCD()->initLCD();
  getLCD()->clear();
  getLCD()->setCursor(0, 0);
  getLCD()->writeString("Hello, World!");
}

} // namespace RaspberryPi
} // namespace BSP
} // namespace Driver
