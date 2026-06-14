#pragma once

#include <driver/driver.hpp>
#include <stdint.h>

namespace Driver::Display {

enum class Rotation {
  Deg0,
  Deg90,
  Deg180,
  Deg270,
};

class PixelDisplay : public Driver::DeviceDriver {
public:
  virtual unsigned int width() const = 0;
  virtual unsigned int height() const = 0;
  virtual void setRotation(Rotation rotation) = 0;
  virtual void fillScreen(uint16_t color) = 0;
  virtual void fillRect(unsigned int x, unsigned int y, unsigned int width,
                        unsigned int height, uint16_t color) = 0;
  virtual void blitRgb565(unsigned int x, unsigned int y, unsigned int width,
                          unsigned int height,
                          const uint16_t *pixels) = 0;

protected:
  PixelDisplay() = default;
  ~PixelDisplay() = default;
};

} // namespace Driver::Display
