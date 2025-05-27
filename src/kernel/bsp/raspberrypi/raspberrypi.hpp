#pragma once

#include "../device_driver/bcm/bcm2xxx_gpio.hpp"
#include "../device_driver/bcm/bcm2xxx_pl011_uart.hpp"
#include "../device_driver/bcm/bcm2xxx_rng.hpp"
#include "../device_driver/lcd/hd44780u.hpp"
#include <bsp/bsp.hpp>

namespace Driver {
namespace BSP {
namespace RaspberryPi {

class RaspberryPi {
public:
  static void init();
  static Driver::BSP::BCM::GPIO *getGPIO();
  static Driver::BSP::BCM::UART *getUART();
  static Driver::BSP::BCM::UART::UartConsole *getUartConsole();
  static Driver::BSP::LCD::HD44780U *getLCD();
  static Driver::BSP::BCM::RNG *getRNG();

private:
  static Driver::BSP::BCM::GPIO *gpio;
  static Driver::BSP::BCM::UART *uart;
  static Driver::BSP::BCM::UART::UartConsole *uartConsole;
  static Driver::BSP::LCD::HD44780U *lcd;
  static Driver::BSP::BCM::RNG *rng;
  static void postInitUart();
  static void postInitGpio();
  static void postInitLCD();
  static void postInitRng();
};

} // namespace RaspberryPi
} // namespace BSP
} // namespace Driver
