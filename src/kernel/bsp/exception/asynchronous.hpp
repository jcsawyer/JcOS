#pragma once

#include <bsp/device_driver/common.hpp>
#include <stdint.h>

namespace BSP::Exception::Asynchronous {

using LocalIRQ = BSP::Common::BoundedUsize<3>;
using PeripheralIRQ = BSP::Common::BoundedUsize<63>;

enum class IRQKind {
  Local,
  Peripheral,
};

struct IRQNumber {
  IRQKind kind;
  LocalIRQ local;
  PeripheralIRQ peripheral;

  static IRQNumber local_irq(uint32_t irq) {
    return IRQNumber{IRQKind::Local, LocalIRQ::newBounded(irq).value(),
                     PeripheralIRQ::newBounded(0).value()};
  }

  static IRQNumber peripheral_irq(uint32_t irq) {
    return IRQNumber{IRQKind::Peripheral, LocalIRQ::newBounded(0).value(),
                     PeripheralIRQ::newBounded(irq).value()};
  }
};

} // namespace BSP::Exception::Asynchronous
