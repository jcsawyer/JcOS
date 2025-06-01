#pragma once

#include <bsp/device_driver/common.hpp>
#include <stdint.h>

namespace BSP::Exception::Asynchronous {

using PeripheralIRQ = BSP::Common::BoundedUsize<63>;

enum class IRQKind {
  Local,
  Peripheral,
};

struct IRQNumber {
  IRQKind kind;
  PeripheralIRQ peripheral;

  static IRQNumber peripheral_irq(uint32_t irq) {
    return IRQNumber{IRQKind::Peripheral,
                     PeripheralIRQ::newBounded(irq).value()};
  }
};

} // namespace BSP::Exception::Asynchronous
