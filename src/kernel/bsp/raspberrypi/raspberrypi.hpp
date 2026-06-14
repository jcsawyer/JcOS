#pragma once

#include "../device_driver/bcm/bcm2xxx_gpio.hpp"
#include "../device_driver/bcm/bcm2xxx_interrupt_controller.hpp"
#include "../device_driver/bcm/bcm2xxx_pl011_uart.hpp"
#include "../device_driver/bcm/bcm2xxx_rng.hpp"
#include "../device_driver/bcm/bcm2xxx_spi.hpp"
#include "../device_driver/bcm/bcm2xxx_timer.hpp"
#include "../device_driver/display/spi_tft_display.hpp"
#include "../device_driver/lcd/hd44780u.hpp"
#include <bsp/bsp.hpp>

namespace Driver {
namespace BSP {
namespace RaspberryPi {

struct HD44780Pins {
  unsigned char registerSelect;
  unsigned char enable;
  unsigned char d4;
  unsigned char d5;
  unsigned char d6;
  unsigned char d7;
};

struct TFTPins {
  unsigned int reset;
  unsigned int dataCommand;
};

struct TouchPins {
  unsigned int sda;
  unsigned int scl;
  unsigned int reset;
  unsigned int irq;
};

struct BoardPins {
  HD44780Pins lcd;
  TFTPins tft;
  TouchPins touch;
};

class RaspberryPi {
public:
  static void init();
  static const BoardPins &boardPins();
  static Driver::BSP::BCM::GPIO *getGPIO();
  static Driver::BSP::BCM::SPI *getSPI();
  static Driver::BSP::BCM::UART *getUART();
  static Driver::BSP::BCM::UART::UartConsole *getUartConsole();
  static Driver::BSP::LCD::HD44780U *getLCD();
  static Driver::BSP::Display::SPITFTDisplay *getTftDisplay();
  static Driver::BSP::BCM::RNG *getRNG();
  static Driver::BSP::BCM::Timer *getTimer();
  static Driver::BSP::BCM::InterruptController *getInterruptController();

private:
  static Driver::BSP::BCM::GPIO *gpio;
  static Driver::BSP::BCM::SPI *spi;
  static Driver::BSP::BCM::UART *uart;
  static Driver::BSP::BCM::UART::UartConsole *uartConsole;
  static Driver::BSP::LCD::HD44780U *lcd;
  static Driver::BSP::Display::SPITFTDisplay *tftDisplay;
  static Driver::BSP::BCM::RNG *rng;
  static Driver::BSP::BCM::Timer *timer;
  static Driver::BSP::BCM::InterruptController *interruptController;
  static void postInitUart();
  static void postInitGpio();
  static void postInitSpi();
  static void postInitLCD();
  static void postInitTftDisplay();
  static void postInitRng();
  static void postInitTimer();
  static void postInitInterruptController();
};

} // namespace RaspberryPi
} // namespace BSP
} // namespace Driver
