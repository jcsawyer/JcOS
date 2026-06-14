#pragma once

#include <bsp/device_driver/bcm/bcm2xxx_gpio.hpp>
#include <bsp/exception/asynchronous.hpp>
#include <driver/driver.hpp>
#include <spi/spi.hpp>
#include <stdint.h>

namespace Driver::BSP::BCM {

class SPI : public Driver::DeviceDriver {
public:
  explicit SPI(uintptr_t mmio_start_addr) : registerBlock(mmio_start_addr) {}

  const char *compatible() override { return "BCM SPI0"; }
  void init() override;
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;

  void configure(const Driver::SPI::DeviceConfig &config) const;
  void beginTransfer(const Driver::SPI::DeviceConfig &config) const;
  void endTransfer(const Driver::SPI::DeviceConfig &config) const;
  void write(const uint8_t *data, size_t length) const;
  uint8_t transfer(uint8_t value) const;

private:
  static constexpr uint32_t CS_LEN_LONG = 1u << 25;
  static constexpr uint32_t CS_DMA_LEN = 1u << 24;
  static constexpr uint32_t CS_CSPOL2 = 1u << 23;
  static constexpr uint32_t CS_CSPOL1 = 1u << 22;
  static constexpr uint32_t CS_CSPOL0 = 1u << 21;
  static constexpr uint32_t CS_RXF = 1u << 20;
  static constexpr uint32_t CS_RXR = 1u << 19;
  static constexpr uint32_t CS_TXD = 1u << 18;
  static constexpr uint32_t CS_RXD = 1u << 17;
  static constexpr uint32_t CS_DONE = 1u << 16;
  static constexpr uint32_t CS_TE_EN = 1u << 15;
  static constexpr uint32_t CS_LMONO = 1u << 14;
  static constexpr uint32_t CS_LEN = 1u << 13;
  static constexpr uint32_t CS_REN = 1u << 12;
  static constexpr uint32_t CS_ADCS = 1u << 11;
  static constexpr uint32_t CS_INTR = 1u << 10;
  static constexpr uint32_t CS_INTD = 1u << 9;
  static constexpr uint32_t CS_DMAEN = 1u << 8;
  static constexpr uint32_t CS_TA = 1u << 7;
  static constexpr uint32_t CS_CLEAR_RX = 1u << 5;
  static constexpr uint32_t CS_CLEAR_TX = 1u << 4;
  static constexpr uint32_t CS_CPOL = 1u << 3;
  static constexpr uint32_t CS_CPHA = 1u << 2;
  static constexpr uint32_t CS_CS_MASK = 0b11u;

  class RegisterBlock {
  public:
    volatile uint32_t *CS;
    volatile uint32_t *FIFO;
    volatile uint32_t *CLK;
    volatile uint32_t *DLEN;
    volatile uint32_t *LTOH;
    volatile uint32_t *DC;

    explicit RegisterBlock(uintptr_t mmio_start_addr) {
      CS = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x00);
      FIFO = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x04);
      CLK = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x08);
      DLEN = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x0C);
      LTOH = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x10);
      DC = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x14);
    }
  };

  RegisterBlock registerBlock;

  uint32_t buildControlWord(const Driver::SPI::DeviceConfig &config) const;
  void clearFifos() const;
};

} // namespace Driver::BSP::BCM
