#include "raspberrypi.hpp"
#include "../../driver/driver.hpp"
#include "../../std/stddef.h"
#include "memory.hpp"

namespace Driver {
namespace BSP {
namespace RaspberryPi {

Driver::BSP::BCM::GPIO *RaspberryPi::gpio = nullptr;
Driver::BSP::BCM::UART *RaspberryPi::uart = nullptr;
Driver::BSP::BCM::RNG *RaspberryPi::rng = nullptr;
Driver::BSP::BCM::UART::UartConsole *RaspberryPi::uartConsole = nullptr;

Driver::BSP::BCM::GPIO *RaspberryPi::getGPIO() {
  if (gpio == nullptr) {
    static Driver::BSP::BCM::GPIO gpioInstance(
        Memory::Map::getMMIO().GPIO_START);
    gpio = &gpioInstance;
  }
  return gpio;
}

Driver::BSP::BCM::UART *RaspberryPi::getUART() {
  if (uart == nullptr) {
    static Driver::BSP::BCM::UART uartInstance(
        Memory::Map::getMMIO().PL011_UART_START);
    uart = &uartInstance;
  }
  return uart;
}

Driver::BSP::BCM::RNG *RaspberryPi::getRNG() {
  if (rng == nullptr) {
    static Driver::BSP::BCM::RNG rngInstance(Memory::Map::getMMIO().RNG_START);
    rng = &rngInstance;
  }
  return rng;
}

Driver::BSP::BCM::UART::UartConsole *RaspberryPi::getUartConsole() {
  if (uartConsole == nullptr) {
    static Driver::BSP::BCM::UART::UartConsole uartConsoleInstance(getUART());
    uartConsole = &uartConsoleInstance;
  }
  return uartConsole;
}

void RaspberryPi::init() {
  Driver::driverManager().addDriver(
      Driver::DeviceDriverDescriptor(getGPIO(), &postInitGpio));
  Driver::driverManager().addDriver(
      Driver::DeviceDriverDescriptor(getUART(), &postInitUart));
  Driver::driverManager().addDriver(
      Driver::DeviceDriverDescriptor(getRNG(), &postInitRng));
}

void RaspberryPi::postInitUart() {
  Console::Console::SetInstance(getUartConsole());
}

void RaspberryPi::postInitGpio() { getGPIO()->mapPl011Uart(); }

void RaspberryPi::postInitRng() {}

} // namespace RaspberryPi
} // namespace BSP
} // namespace Driver
