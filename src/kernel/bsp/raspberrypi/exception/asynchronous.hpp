#pragma once

#include <bsp/exception/asynchronous.hpp>

namespace Driver::BSP::RaspberryPi {

#if BOARD == bsp_rpi3
static inline ::BSP::Exception::Asynchronous::IRQNumber *PL011_UART() {
  static ::BSP::Exception::Asynchronous::IRQNumber pl011_uart =
      ::BSP::Exception::Asynchronous::IRQNumber::peripheral_irq(57);
  return &pl011_uart;
}
#elif BOARD == bsp_rpi4
static ::BSP::Exception::Asynchronous::IRQNumber PL011_UART =
    ::BSP::Exception::Asynchronous::IRQNumber::peripheral_irq(153);
#else
#error Unknown board
#endif

} // namespace Driver::BSP::RaspberryPi