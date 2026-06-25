#pragma once

#include "../../../driver/driver.hpp"
#include <bsp/exception/asynchronous.hpp>
#include <stddef.h>
#include <stdint.h>

namespace Driver::BSP::BCM {

class SPI0 : public Driver::DeviceDriver {
public:
  explicit SPI0(uintptr_t mmio_start_addr) : registerBlock(mmio_start_addr) {}

  const char *compatible() override { return "BCM SPI0"; }
  void init() override;
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;

  void configure(uint16_t clockDivider);
  void beginTransaction();
  void endTransaction();
  void write(const uint8_t *data, size_t length);
  uint8_t transferByte(uint8_t value);

private:
  class RegisterBlock {
  public:
    volatile uint32_t *CS;
    volatile uint32_t *FIFO;
    volatile uint32_t *CLK;

    explicit RegisterBlock(uintptr_t mmio_start_addr) {
      CS = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x00);
      FIFO = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x04);
      CLK = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x08);
    }
  };

  RegisterBlock registerBlock;

  static constexpr uint32_t CS_CLEAR_RX = 1u << 5;
  static constexpr uint32_t CS_CLEAR_TX = 1u << 4;
  static constexpr uint32_t CS_TA = 1u << 7;
  static constexpr uint32_t CS_DONE = 1u << 16;
  static constexpr uint32_t CS_RXD = 1u << 17;
  static constexpr uint32_t CS_TXD = 1u << 18;
  static constexpr uint32_t CS_CS0 = 0u;

  void clearFifos();
  void waitWhileTransferring() const;
};

} // namespace Driver::BSP::BCM
