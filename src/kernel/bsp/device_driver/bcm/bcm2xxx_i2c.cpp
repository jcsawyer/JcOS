#include "bcm2xxx_i2c.hpp"

#include "../../../arch/cpu.hpp"

namespace Driver::BSP::BCM {

void I2C::init() {
  *registerBlock.DIV = coreClockDivider;
  *registerBlock.DEL = 0x30;
  *registerBlock.CLKT = 0x40;
  clearStatus();
  clearFifo();
  *registerBlock.C = controlI2cEnable;
}

void I2C::registerAndEnableIrqHandler(
    ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) {
  (void)irqNumber;
}

bool I2C::write(uint8_t address, const uint8_t *data, size_t length) {
  if (data == nullptr || length == 0) {
    return false;
  }

  *registerBlock.A = address;
  *registerBlock.DLEN = static_cast<uint32_t>(length);
  clearStatus();
  clearFifo();
  *registerBlock.C =
      controlI2cEnable | controlClearFifo | controlStartTransfer;

  size_t offset = 0;
  while (offset < length) {
    uint32_t guard = transferTimeoutCycles;
    while (((*registerBlock.S & statusTransmitData) == 0u) &&
           ((*registerBlock.S & statusDone) == 0u) && guard > 0) {
      CPU::nop();
      --guard;
    }

    if (guard == 0 || hasTransferError()) {
      return false;
    }

    if ((*registerBlock.S & statusDone) != 0u) {
      break;
    }

    *registerBlock.FIFO = data[offset++];
  }

  return waitForDone() && !hasTransferError() && offset == length;
}

bool I2C::read(uint8_t address, uint8_t *buffer, size_t length) {
  if (buffer == nullptr || length == 0) {
    return false;
  }

  *registerBlock.A = address;
  *registerBlock.DLEN = static_cast<uint32_t>(length);
  clearStatus();
  clearFifo();
  *registerBlock.C = controlI2cEnable | controlClearFifo | controlRead |
                     controlStartTransfer;

  size_t offset = 0;
  while (offset < length) {
    uint32_t guard = transferTimeoutCycles;
    while (((*registerBlock.S & statusReceiveData) == 0u) &&
           ((*registerBlock.S & statusDone) == 0u) && guard > 0) {
      CPU::nop();
      --guard;
    }

    if (guard == 0 || hasTransferError()) {
      return false;
    }

    while (((*registerBlock.S & statusReceiveData) != 0u) && offset < length) {
      buffer[offset++] = static_cast<uint8_t>(*registerBlock.FIFO);
    }

    if ((*registerBlock.S & statusDone) != 0u && offset >= length) {
      break;
    }
  }

  return waitForDone() && !hasTransferError() && offset == length;
}

bool I2C::writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
  const uint8_t payload[2] = {reg, value};
  return write(address, payload, sizeof(payload));
}

bool I2C::readRegisters(uint8_t address, uint8_t reg, uint8_t *buffer,
                        size_t length) {
  return write(address, &reg, 1) && read(address, buffer, length);
}

void I2C::clearStatus() const {
  *registerBlock.S =
      statusClockTimeout | statusAcknowledgeError | statusDone;
}

void I2C::clearFifo() const { *registerBlock.C |= controlClearFifo; }

bool I2C::waitForDone() const {
  uint32_t guard = transferTimeoutCycles;
  while (((*registerBlock.S & statusDone) == 0u) && guard > 0) {
    CPU::nop();
    --guard;
  }

  return guard > 0;
}

bool I2C::hasTransferError() const {
  const uint32_t status = *registerBlock.S;
  return (status & (statusClockTimeout | statusAcknowledgeError)) != 0u;
}

} // namespace Driver::BSP::BCM
