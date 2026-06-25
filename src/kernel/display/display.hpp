#pragma once

#include "font5x7.hpp"
#include <driver/driver.hpp>
#include <stddef.h>
#include <stdint.h>

namespace Driver::Display {

class Display : public Driver::DeviceDriver {
public:
  enum class Rotation {
    Portrait,
    Landscape,
    InvertedPortrait,
    InvertedLandscape,
  };

  virtual bool isReady() const = 0;
  virtual unsigned int width() const = 0;
  virtual unsigned int height() const = 0;
  virtual void clear(uint16_t color) = 0;
  virtual void setRotation(Rotation rotation) = 0;
  virtual Rotation rotation() const = 0;
  virtual void setAddressWindow(unsigned int x0, unsigned int y0,
                                unsigned int x1, unsigned int y1) = 0;
  virtual void writePixels(const uint16_t *pixels, size_t count) = 0;

  void drawPixel(unsigned int x, unsigned int y, uint16_t color) {
    if (x >= width() || y >= height()) {
      return;
    }

    setAddressWindow(x, y, x, y);
    writePixels(&color, 1);
  }

  void fillRect(unsigned int x, unsigned int y, unsigned int rectWidth,
                unsigned int rectHeight, uint16_t color) {
    if (rectWidth == 0 || rectHeight == 0 || x >= width() || y >= height()) {
      return;
    }

    if ((x + rectWidth) > width()) {
      rectWidth = width() - x;
    }

    if ((y + rectHeight) > height()) {
      rectHeight = height() - y;
    }

    constexpr size_t chunkSize = 64;
    uint16_t pixelChunk[chunkSize];
    for (size_t i = 0; i < chunkSize; i++) {
      pixelChunk[i] = color;
    }

    setAddressWindow(x, y, x + rectWidth - 1, y + rectHeight - 1);
    const size_t totalPixels = static_cast<size_t>(rectWidth) * rectHeight;
    size_t remaining = totalPixels;

    while (remaining > 0) {
      const size_t currentChunk = remaining > chunkSize ? chunkSize : remaining;
      writePixels(pixelChunk, currentChunk);
      remaining -= currentChunk;
    }
  }

  void drawMonoGlyph(unsigned int x, unsigned int y, char value,
                     uint16_t foreground, uint16_t background,
                     unsigned int scale = 1) {
    const Glyph5x7 &glyph = lookupGlyph5x7(value);

    for (unsigned int row = 0; row < 7; row++) {
      for (unsigned int col = 0; col < 5; col++) {
        const bool pixelOn = (glyph.rows[row] & (1u << (4 - col))) != 0;
        fillRect(x + col * scale, y + row * scale, scale, scale,
                 pixelOn ? foreground : background);
      }
    }
  }

  void drawString(unsigned int x, unsigned int y, const char *text,
                  uint16_t foreground, uint16_t background,
                  unsigned int scale = 1) {
    if (text == nullptr) {
      return;
    }

    const unsigned int glyphAdvance = 6 * scale;
    unsigned int cursorX = x;
    unsigned int cursorY = y;

    while (*text != '\0') {
      if (*text == '\n') {
        cursorX = x;
        cursorY += 8 * scale;
      } else {
        drawMonoGlyph(cursorX, cursorY, *text, foreground, background, scale);
        cursorX += glyphAdvance;
      }
      ++text;
    }
  }
};

} // namespace Driver::Display
