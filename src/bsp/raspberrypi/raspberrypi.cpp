#include "raspberrypi.hpp"
#include "../../std/cstddef.h"
#include "memory.hpp"

namespace Driver {
namespace BSP {
namespace RaspberryPi {

Driver::BSP::BCM::GPIO *Driver::BSP::RaspberryPi::RaspberryPi::gpio = nullptr;
Driver::BSP::BCM::UART *Driver::BSP::RaspberryPi::RaspberryPi::uart = nullptr;
Driver::BSP::BCM::UART::UartConsole
    *Driver::BSP::RaspberryPi::RaspberryPi::uartConsole = nullptr;

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
}

void RaspberryPi::postInitUart() {
  Console::Console::SetInstance(getUartConsole());
  Console::Console::GetInstance()->printLine("UART initialized");
}

void RaspberryPi::postInitGpio() { getGPIO()->mapPl011Uart(); }

} // namespace RaspberryPi
} // namespace BSP
} // namespace Driver
