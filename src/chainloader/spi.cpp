#include "spi.hpp"

#include <arch/cpu.hpp>

namespace Chainloader {

namespace {
constexpr uint32_t CS_CLEAR_RX_TX = 0b11u << 4;
constexpr uint32_t CS_TA = 1u << 7;
constexpr uint32_t CS_DONE = 1u << 16;
constexpr uint32_t CS_RXD = 1u << 17;
constexpr uint32_t CS_TXD = 1u << 18;
constexpr uint32_t SPI_CLOCK_DIVIDER = 32;
} // namespace

SPI::SPI(uintptr_t baseAddress) {
  cs_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x00);
  fifo_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x04);
  clk_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x08);
}

void SPI::init() {
  *cs_ = 0;
  *clk_ = SPI_CLOCK_DIVIDER;
  *cs_ = CS_CLEAR_RX_TX;
}

void SPI::write(const uint8_t *data, size_t length) const {
  if (data == nullptr || length == 0) {
    return;
  }

  *cs_ = CS_CLEAR_RX_TX;
  *cs_ = CS_CLEAR_RX_TX | CS_TA;

  for (size_t i = 0; i < length; ++i) {
    while ((*cs_ & CS_TXD) == 0) {
      CPU::nop();
    }

    *fifo_ = data[i];

    while ((*cs_ & CS_RXD) != 0) {
      (void)*fifo_;
    }
  }

  while ((*cs_ & CS_DONE) == 0) {
    CPU::nop();
  }

  while ((*cs_ & CS_RXD) != 0) {
    (void)*fifo_;
  }

  *cs_ = 0;
}

} // namespace Chainloader
