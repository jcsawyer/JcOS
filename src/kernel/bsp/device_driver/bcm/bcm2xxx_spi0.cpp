#include "bcm2xxx_spi0.hpp"

namespace Driver::BSP::BCM {

void SPI0::init() { configure(16); }

void SPI0::registerAndEnableIrqHandler(
    ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) {
  (void)irqNumber;
}

void SPI0::configure(uint16_t clockDivider) {
  *registerBlock.CLK = clockDivider;
  *registerBlock.CS = CS_CS0;
  clearFifos();
}

void SPI0::beginTransaction() {
  clearFifos();
  *registerBlock.CS = CS_CS0 | CS_TA;
}

void SPI0::endTransaction() {
  waitWhileTransferring();
  *registerBlock.CS = CS_CS0;
  clearFifos();
}

void SPI0::write(const uint8_t *data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    (void)transferByte(data[i]);
  }
}

uint8_t SPI0::transferByte(uint8_t value) {
  while ((*registerBlock.CS & CS_TXD) == 0u) {
  }

  *registerBlock.FIFO = value;

  while ((*registerBlock.CS & CS_RXD) == 0u) {
  }

  return static_cast<uint8_t>(*registerBlock.FIFO);
}

void SPI0::clearFifos() { *registerBlock.CS |= CS_CLEAR_RX | CS_CLEAR_TX; }

void SPI0::waitWhileTransferring() const {
  while ((*registerBlock.CS & CS_DONE) == 0u) {
  }
}

} // namespace Driver::BSP::BCM
