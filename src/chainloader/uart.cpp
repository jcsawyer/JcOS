#include "uart.hpp"

#include "board_config.hpp"

#include <arch/cpu.hpp>

namespace Chainloader {

Uart::Uart(uintptr_t baseAddress) {
  dr_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x00);
  fr_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x18);
  ibrd_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x24);
  fbrd_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x28);
  lcrh_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x2C);
  cr_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x30);
  ifls_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x34);
  imsc_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x38);
  icr_ = reinterpret_cast<volatile uint32_t *>(baseAddress + 0x44);
}

void Uart::init() {
  *cr_ = 0;
  *icr_ = 0x7FF;
  *ibrd_ = Board::UART_IBRD;
  *fbrd_ = Board::UART_FBRD;
  *lcrh_ = (1u << 4) | (0b11u << 5);
  *ifls_ = 0;
  *imsc_ = 0;
  *cr_ = (1u << 0) | (1u << 8) | (1u << 9);
}

void Uart::writeByte(char value) const {
  while ((*fr_ & 0x20u) != 0) {
    CPU::nop();
  }
  *dr_ = static_cast<uint32_t>(value);
}

void Uart::writeBuffer(const void *buffer, size_t length) const {
  const char *bytes = static_cast<const char *>(buffer);
  for (size_t i = 0; i < length; ++i) {
    writeByte(bytes[i]);
  }
}

bool Uart::readByte(char &value) const {
  if ((*fr_ & 0x10u) != 0) {
    return false;
  }

  value = static_cast<char>(*dr_ & 0xFFu);
  return true;
}

void Uart::clearRx() const {
  char discarded = 0;
  while (readByte(discarded)) {
  }
}

} // namespace Chainloader
