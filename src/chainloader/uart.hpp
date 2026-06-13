#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Chainloader {

class Uart {
public:
  explicit Uart(uintptr_t baseAddress);

  void init();
  void writeByte(char value) const;
  void writeBuffer(const void *buffer, size_t length) const;
  bool readByte(char &value) const;
  void clearRx() const;

private:
  volatile uint32_t *dr_;
  volatile uint32_t *fr_;
  volatile uint32_t *ibrd_;
  volatile uint32_t *fbrd_;
  volatile uint32_t *lcrh_;
  volatile uint32_t *cr_;
  volatile uint32_t *ifls_;
  volatile uint32_t *imsc_;
  volatile uint32_t *icr_;
};

} // namespace Chainloader
