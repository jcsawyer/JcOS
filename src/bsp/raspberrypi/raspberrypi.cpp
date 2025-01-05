#include "memory.hpp"
#include "raspberrypi_driver.hpp"

namespace Driver {
namespace BSP {
namespace RaspberryPi {
static Driver::BSP::BCM::GPIO gpio =
    Driver::BSP::BCM::GPIO(Memory::Map::getMMIO().GPIO_START);
static Driver::BSP::BCM::UART uart =
    Driver::BSP::BCM::UART(Memory::Map::getMMIO().PL011_UART_START);
static Driver::BSP::BCM::UART::UartConsole uartConsole =
    Driver::BSP::BCM::UART::UartConsole(&uart);

void postInitUart() { Console::setConsole(&uartConsole); }
void posrtInitGpio() { gpio.mapPl011Uart(); }

void init() {
  Driver::driverManager().addDriver(
      Driver::DeviceDriverDescriptor(&gpio, &posrtInitGpio));
  Driver::driverManager().addDriver(
      Driver::DeviceDriverDescriptor(&uart, &postInitUart));
}
} // namespace RaspberryPi
} // namespace BSP
} // namespace Driver
