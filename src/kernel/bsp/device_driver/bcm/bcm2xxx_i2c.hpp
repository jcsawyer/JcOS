#pragma once

#include "../../../driver/driver.hpp"
#include <bsp/exception/asynchronous.hpp>
#include <stddef.h>
#include <stdint.h>

namespace Driver::BSP::BCM {

class I2C : public Driver::DeviceDriver {
public:
  explicit I2C(uintptr_t mmio_start_addr) : registerBlock(mmio_start_addr) {}

  const char *compatible() override { return "BCM I2C1"; }
  void init() override;
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;

  bool write(uint8_t address, const uint8_t *data, size_t length);
  bool read(uint8_t address, uint8_t *buffer, size_t length);
  bool writeRegister(uint8_t address, uint8_t reg, uint8_t value);
  bool readRegisters(uint8_t address, uint8_t reg, uint8_t *buffer,
                     size_t length);

private:
  class RegisterBlock {
  public:
    volatile uint32_t *C;
    volatile uint32_t *S;
    volatile uint32_t *DLEN;
    volatile uint32_t *A;
    volatile uint32_t *FIFO;
    volatile uint32_t *DIV;
    volatile uint32_t *DEL;
    volatile uint32_t *CLKT;

    explicit RegisterBlock(uintptr_t mmio_start_addr) {
      C = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x00);
      S = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x04);
      DLEN = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x08);
      A = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x0C);
      FIFO = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x10);
      DIV = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x14);
      DEL = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x18);
      CLKT = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x1C);
    }
  };

  RegisterBlock registerBlock;

  static constexpr uint32_t controlI2cEnable = 1u << 15;
  static constexpr uint32_t controlStartTransfer = 1u << 7;
  static constexpr uint32_t controlClearFifo = 0b11u << 4;
  static constexpr uint32_t controlRead = 1u << 0;

  static constexpr uint32_t statusClockTimeout = 1u << 9;
  static constexpr uint32_t statusAcknowledgeError = 1u << 8;
  static constexpr uint32_t statusReceiveData = 1u << 5;
  static constexpr uint32_t statusTransmitData = 1u << 4;
  static constexpr uint32_t statusDone = 1u << 1;

  static constexpr uint32_t coreClockDivider = 2500;
  static constexpr uint32_t transferTimeoutCycles = 2000000;

  void clearStatus() const;
  void clearFifo() const;
  bool waitForDone() const;
  bool hasTransferError() const;
};

} // namespace Driver::BSP::BCM
