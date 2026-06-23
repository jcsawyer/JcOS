#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Chainloader {

class SPI {
public:
  explicit SPI(uintptr_t baseAddress);

  void init();
  void write(const uint8_t *data, size_t length) const;

private:
  volatile uint32_t *cs_;
  volatile uint32_t *fifo_;
  volatile uint32_t *clk_;
};

} // namespace Chainloader
