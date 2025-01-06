#pragma once

#include "../device_driver/bcm/bcm2xxx_gpio.hpp"
#include "../device_driver/bcm/bcm2xxx_pl011_uart.hpp"
#include "../device_driver/bcm/bcm2xxx_rng.hpp"

namespace Driver {
namespace BSP {
namespace RaspberryPi {

class RaspberryPi {
public:
  static void init();
  static Driver::BSP::BCM::GPIO *getGPIO();
  static Driver::BSP::BCM::UART *getUART();
  static Driver::BSP::BCM::RNG *getRNG();
  static Driver::BSP::BCM::UART::UartConsole *getUartConsole();

private:
  RaspberryPi() = default;
  static Driver::BSP::BCM::GPIO *gpio;
  static Driver::BSP::BCM::UART *uart;
  static Driver::BSP::BCM::RNG *rng;
  static Driver::BSP::BCM::UART::UartConsole *uartConsole;
  static void postInitUart();
  static void postInitGpio();
  static void postInitRng();
};

} // namespace RaspberryPi
} // namespace BSP
} // namespace Driver
