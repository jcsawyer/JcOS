#include "boot_display.hpp"

#include "board_config.hpp"

#include <stdarg.h>
#include <stdio/printf.h>

namespace Chainloader {

namespace {
constexpr uint16_t kBackgroundColor = Board::TFT_BACKGROUND;
constexpr uint16_t kForegroundColor = Board::TFT_FOREGROUND;
constexpr uint16_t kAccentColor = 0x07E0;
constexpr uint16_t kErrorColor = 0xF800;

constexpr unsigned int kFontWidth = 5;
constexpr unsigned int kFontHeight = 7;
constexpr unsigned int kFontSpacing = 1;

constexpr const char *kBootLogo = R"(  __
  |  |
  |  |    ----- -----
  |  |---|     |     |
--|  |  | | |  |  -- |
|_____|__|___|_____|)";

size_t stringLength(const char *text) {
  if (text == nullptr) {
    return 0;
  }

  size_t length = 0;
  while (text[length] != '\0') {
    ++length;
  }
  return length;
}

void formatString(char *buffer, size_t bufferSize, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vsnprintf_(buffer, bufferSize, format, args);
  va_end(args);
}

const uint8_t *glyphFor(char character) {
#define GLYPH_CASE(value, ...)                                                 \
  case value: {                                                                \
    static const uint8_t glyph[] = {__VA_ARGS__};                              \
    return glyph;                                                              \
  }

  switch (character) {
    GLYPH_CASE(' ', 0x00, 0x00, 0x00, 0x00, 0x00)
    GLYPH_CASE('-', 0x08, 0x08, 0x08, 0x08, 0x08)
    GLYPH_CASE('.', 0x00, 0x60, 0x60, 0x00, 0x00)
    GLYPH_CASE('/', 0x40, 0x20, 0x10, 0x08, 0x04)
    GLYPH_CASE('%', 0x63, 0x13, 0x08, 0x64, 0x63)
    GLYPH_CASE('0', 0x3E, 0x51, 0x49, 0x45, 0x3E)
    GLYPH_CASE('1', 0x00, 0x42, 0x7F, 0x40, 0x00)
    GLYPH_CASE('2', 0x42, 0x61, 0x51, 0x49, 0x46)
    GLYPH_CASE('3', 0x21, 0x41, 0x45, 0x4B, 0x31)
    GLYPH_CASE('4', 0x18, 0x14, 0x12, 0x7F, 0x10)
    GLYPH_CASE('5', 0x27, 0x45, 0x45, 0x45, 0x39)
    GLYPH_CASE('6', 0x3C, 0x4A, 0x49, 0x49, 0x30)
    GLYPH_CASE('7', 0x01, 0x71, 0x09, 0x05, 0x03)
    GLYPH_CASE('8', 0x36, 0x49, 0x49, 0x49, 0x36)
    GLYPH_CASE('9', 0x06, 0x49, 0x49, 0x29, 0x1E)
    GLYPH_CASE('A', 0x7E, 0x11, 0x11, 0x11, 0x7E)
    GLYPH_CASE('B', 0x7F, 0x49, 0x49, 0x49, 0x36)
    GLYPH_CASE('C', 0x3E, 0x41, 0x41, 0x41, 0x22)
    GLYPH_CASE('D', 0x7F, 0x41, 0x41, 0x22, 0x1C)
    GLYPH_CASE('E', 0x7F, 0x49, 0x49, 0x49, 0x41)
    GLYPH_CASE('F', 0x7F, 0x09, 0x09, 0x09, 0x01)
    GLYPH_CASE('G', 0x3E, 0x41, 0x49, 0x49, 0x7A)
    GLYPH_CASE('H', 0x7F, 0x08, 0x08, 0x08, 0x7F)
    GLYPH_CASE('I', 0x00, 0x41, 0x7F, 0x41, 0x00)
    GLYPH_CASE('J', 0x20, 0x40, 0x41, 0x3F, 0x01)
    GLYPH_CASE('K', 0x7F, 0x08, 0x14, 0x22, 0x41)
    GLYPH_CASE('L', 0x7F, 0x40, 0x40, 0x40, 0x40)
    GLYPH_CASE('M', 0x7F, 0x02, 0x0C, 0x02, 0x7F)
    GLYPH_CASE('N', 0x7F, 0x04, 0x08, 0x10, 0x7F)
    GLYPH_CASE('O', 0x3E, 0x41, 0x41, 0x41, 0x3E)
    GLYPH_CASE('P', 0x7F, 0x09, 0x09, 0x09, 0x06)
    GLYPH_CASE('Q', 0x3E, 0x41, 0x51, 0x21, 0x5E)
    GLYPH_CASE('R', 0x7F, 0x09, 0x19, 0x29, 0x46)
    GLYPH_CASE('S', 0x46, 0x49, 0x49, 0x49, 0x31)
    GLYPH_CASE('T', 0x01, 0x01, 0x7F, 0x01, 0x01)
    GLYPH_CASE('U', 0x3F, 0x40, 0x40, 0x40, 0x3F)
    GLYPH_CASE('V', 0x1F, 0x20, 0x40, 0x20, 0x1F)
    GLYPH_CASE('W', 0x3F, 0x40, 0x38, 0x40, 0x3F)
    GLYPH_CASE('X', 0x63, 0x14, 0x08, 0x14, 0x63)
    GLYPH_CASE('Y', 0x07, 0x08, 0x70, 0x08, 0x07)
    GLYPH_CASE('Z', 0x61, 0x51, 0x49, 0x45, 0x43)
    GLYPH_CASE('_', 0x40, 0x40, 0x40, 0x40, 0x40)
    GLYPH_CASE('a', 0x20, 0x54, 0x54, 0x54, 0x78)
    GLYPH_CASE('b', 0x7F, 0x48, 0x44, 0x44, 0x38)
    GLYPH_CASE('c', 0x38, 0x44, 0x44, 0x44, 0x20)
    GLYPH_CASE('d', 0x38, 0x44, 0x44, 0x48, 0x7F)
    GLYPH_CASE('e', 0x38, 0x54, 0x54, 0x54, 0x18)
    GLYPH_CASE('f', 0x08, 0x7E, 0x09, 0x01, 0x02)
    GLYPH_CASE('g', 0x0C, 0x52, 0x52, 0x52, 0x3E)
    GLYPH_CASE('h', 0x7F, 0x08, 0x04, 0x04, 0x78)
    GLYPH_CASE('i', 0x00, 0x44, 0x7D, 0x40, 0x00)
    GLYPH_CASE('j', 0x20, 0x40, 0x44, 0x3D, 0x00)
    GLYPH_CASE('k', 0x7F, 0x10, 0x28, 0x44, 0x00)
    GLYPH_CASE('l', 0x00, 0x41, 0x7F, 0x40, 0x00)
    GLYPH_CASE('m', 0x7C, 0x04, 0x18, 0x04, 0x78)
    GLYPH_CASE('n', 0x7C, 0x08, 0x04, 0x04, 0x78)
    GLYPH_CASE('o', 0x38, 0x44, 0x44, 0x44, 0x38)
    GLYPH_CASE('p', 0x7C, 0x14, 0x14, 0x14, 0x08)
    GLYPH_CASE('q', 0x08, 0x14, 0x14, 0x18, 0x7C)
    GLYPH_CASE('r', 0x7C, 0x08, 0x04, 0x04, 0x08)
    GLYPH_CASE('s', 0x48, 0x54, 0x54, 0x54, 0x20)
    GLYPH_CASE('t', 0x04, 0x3F, 0x44, 0x40, 0x20)
    GLYPH_CASE('u', 0x3C, 0x40, 0x40, 0x20, 0x7C)
    GLYPH_CASE('v', 0x1C, 0x20, 0x40, 0x20, 0x1C)
    GLYPH_CASE('w', 0x3C, 0x40, 0x30, 0x40, 0x3C)
    GLYPH_CASE('x', 0x44, 0x28, 0x10, 0x28, 0x44)
    GLYPH_CASE('y', 0x0C, 0x50, 0x50, 0x50, 0x3C)
    GLYPH_CASE('z', 0x44, 0x64, 0x54, 0x4C, 0x44)
    GLYPH_CASE('|', 0x00, 0x00, 0x7F, 0x00, 0x00)
  default:
    static const uint8_t fallbackGlyph[] = {0x7F, 0x41, 0x5D, 0x41, 0x7F};
    return fallbackGlyph;
  }

#undef GLYPH_CASE
}
} // namespace

BootDisplay::BootDisplay(TFTDisplay &display)
    : display_(display), initialized_(false), statusAreaX_(40),
      statusAreaY_(220), statusAreaWidth_(240), statusAreaHeight_(24),
      detailAreaX_(40), detailAreaY_(254), detailAreaWidth_(240),
      detailAreaHeight_(18), progressBarX_(40), progressBarY_(292),
      progressBarWidth_(240), progressBarHeight_(18), progressFillWidth_(0) {}

void BootDisplay::init() {
  display_.init();
  drawStaticUi();
  initialized_ = true;
}

void BootDisplay::showStage(const char *status, const char *detail) {
  if (!initialized_) {
    return;
  }

  clearStatusArea();
  clearDetailArea();
  setProgress(0);
  drawTextCentered(statusAreaY_, status == nullptr ? "" : status, 2,
                   kForegroundColor);
  if (detail != nullptr && detail[0] != '\0') {
    drawTextCentered(detailAreaY_, detail, 1, kAccentColor);
  }
}

void BootDisplay::showTransferStart(uint32_t totalBytes) {
  char detail[32];
  char sizeBuffer[8];

  formatBytes(sizeBuffer, sizeof(sizeBuffer), totalBytes);
  formatString(detail, sizeof(detail), "0%% of %s", sizeBuffer);
  showStage("Receiving kernel", detail);
}

void BootDisplay::updateTransferProgress(uint32_t receivedBytes,
                                         uint32_t totalBytes) {
  if (!initialized_) {
    return;
  }

  unsigned int percent = 0;
  if (totalBytes != 0) {
    percent = static_cast<unsigned int>(
        (static_cast<uint64_t>(receivedBytes) * 100u) / totalBytes);
  }

  char detail[32];
  char sizeBuffer[8];
  formatBytes(sizeBuffer, sizeof(sizeBuffer), totalBytes);
  formatString(detail, sizeof(detail), "%u%% of %s", percent, sizeBuffer);

  clearDetailArea();
  drawTextCentered(detailAreaY_, detail, 1, kAccentColor);
  setProgress(percent);
}

void BootDisplay::showError(const char *code, const char *detail) {
  char line[40];
  if (detail == nullptr || detail[0] == '\0') {
    formatString(line, sizeof(line), "ERR/%s", code);
  } else {
    formatString(line, sizeof(line), "ERR/%s/%s", code, detail);
  }

  clearStatusArea();
  clearDetailArea();
  setProgress(0);
  drawTextCentered(statusAreaY_, "Chainload error", 2, kErrorColor);
  drawTextCentered(detailAreaY_, line, 1, kErrorColor);
}

void BootDisplay::drawStaticUi() {
  display_.fillScreen(kBackgroundColor);
  drawCenteredLogo(80, kBootLogo, 2, kForegroundColor);
  drawTextCentered(188, "Version 0.1.0", 2, kForegroundColor);

  display_.fillRect(progressBarX_ - 2, progressBarY_ - 2, progressBarWidth_ + 4,
                    progressBarHeight_ + 4, kForegroundColor);
  display_.fillRect(progressBarX_, progressBarY_, progressBarWidth_,
                    progressBarHeight_, kBackgroundColor);
}

void BootDisplay::clearStatusArea() {
  display_.fillRect(statusAreaX_, statusAreaY_, statusAreaWidth_,
                    statusAreaHeight_, kBackgroundColor);
}

void BootDisplay::clearDetailArea() {
  display_.fillRect(detailAreaX_, detailAreaY_, detailAreaWidth_,
                    detailAreaHeight_, kBackgroundColor);
}

void BootDisplay::drawTextCentered(unsigned int y, const char *text,
                                   unsigned int scale, uint16_t color) {
  const unsigned int textWidth =
      static_cast<unsigned int>(stringLength(text)) * glyphAdvance(scale);
  unsigned int x = 0;
  if (display_.width() > textWidth) {
    x = (display_.width() - textWidth) / 2;
  }
  drawText(x, y, text, scale, color);
}

void BootDisplay::drawText(unsigned int x, unsigned int y, const char *text,
                           unsigned int scale, uint16_t color) {
  if (text == nullptr) {
    return;
  }

  unsigned int cursorX = x;
  for (size_t i = 0; text[i] != '\0'; ++i) {
    drawGlyph(cursorX, y, text[i], scale, color);
    cursorX += glyphAdvance(scale);
  }
}

void BootDisplay::drawGlyph(unsigned int x, unsigned int y, char character,
                            unsigned int scale, uint16_t color) {
  if (scale == 0) {
    return;
  }

  const uint8_t *glyph = glyphFor(character);
  for (unsigned int column = 0; column < kFontWidth; ++column) {
    const uint8_t bits = glyph[column];
    for (unsigned int row = 0; row < kFontHeight; ++row) {
      if ((bits & (1u << row)) == 0) {
        continue;
      }

      display_.fillRect(x + column * scale, y + row * scale, scale, scale,
                        color);
    }
  }
}

void BootDisplay::drawLogoGlyph(unsigned int x, unsigned int y, char character,
                                unsigned int scale, uint16_t color) {
  const unsigned int cellWidth = glyphAdvance(scale);
  const unsigned int cellHeight = lineAdvance(scale);
  const unsigned int strokeThickness = scale;

  switch (character) {
  case ' ':
    return;
  case '|': {
    const unsigned int strokeX = x + (cellWidth - strokeThickness) / 2;
    display_.fillRect(strokeX, y, strokeThickness, cellHeight - scale, color);
    return;
  }
  case '-': {
    const unsigned int strokeY = y + (cellHeight - strokeThickness) / 2;
    display_.fillRect(x + scale / 2, strokeY, cellWidth - scale,
                      strokeThickness, color);
    return;
  }
  case '_': {
    const unsigned int strokeY = y + cellHeight - strokeThickness - scale;
    display_.fillRect(x + scale / 2, strokeY, cellWidth - scale,
                      strokeThickness, color);
    return;
  }
  default:
    drawGlyph(x, y, character, scale, color);
    return;
  }
}

void BootDisplay::drawCenteredLogo(unsigned int top, const char *logo,
                                   unsigned int scale, uint16_t color) {
  const unsigned int logoWidth = longestLineLength(logo) * glyphAdvance(scale);
  unsigned int left = 0;
  if (display_.width() > logoWidth) {
    left = (display_.width() - logoWidth) / 2;
  }

  unsigned int y = top;
  size_t lineStart = 0;
  const size_t logoLength = stringLength(logo);
  while (lineStart <= logoLength) {
    size_t lineEnd = lineStart;
    while (lineEnd < logoLength && logo[lineEnd] != '\n') {
      ++lineEnd;
    }

    unsigned int x = left;
    for (size_t i = lineStart; i < lineEnd; ++i) {
      drawLogoGlyph(x, y, logo[i], scale, color);
      x += glyphAdvance(scale);
    }

    y += lineAdvance(scale);
    if (lineEnd == logoLength) {
      break;
    }
    lineStart = lineEnd + 1;
  }
}

void BootDisplay::setProgress(unsigned int percent) {
  if (percent > 100u) {
    percent = 100u;
  }

  const unsigned int innerWidth = progressBarWidth_;
  const unsigned int fillWidth = (innerWidth * percent) / 100u;
  if (fillWidth > progressFillWidth_) {
    display_.fillRect(progressBarX_ + progressFillWidth_, progressBarY_,
                      fillWidth - progressFillWidth_, progressBarHeight_,
                      kAccentColor);
  } else if (fillWidth < progressFillWidth_) {
    display_.fillRect(progressBarX_ + fillWidth, progressBarY_,
                      progressFillWidth_ - fillWidth, progressBarHeight_,
                      kBackgroundColor);
  }
  progressFillWidth_ = fillWidth;
}

unsigned int BootDisplay::glyphAdvance(unsigned int scale) const {
  return (kFontWidth + kFontSpacing) * scale;
}

unsigned int BootDisplay::lineAdvance(unsigned int scale) const {
  return (kFontHeight + kFontSpacing) * scale;
}

unsigned int BootDisplay::longestLineLength(const char *text) const {
  if (text == nullptr || text[0] == '\0') {
    return 0;
  }

  unsigned int longest = 0;
  unsigned int current = 0;
  for (size_t i = 0;; ++i) {
    const char character = text[i];
    if (character == '\n' || character == '\0') {
      if (current > longest) {
        longest = current;
      }
      if (character == '\0') {
        break;
      }
      current = 0;
      continue;
    }
    ++current;
  }

  return longest;
}

void BootDisplay::formatBytes(char *buffer, size_t bufferSize,
                              uint32_t bytes) const {
  if (bytes >= 1024u * 1024u) {
    snprintf_(buffer, bufferSize, "%luM",
              static_cast<unsigned long>((bytes + (1024u * 1024u - 1u)) /
                                         (1024u * 1024u)));
    return;
  }

  snprintf_(buffer, bufferSize, "%luK",
            static_cast<unsigned long>((bytes + 1023u) / 1024u));
}

} // namespace Chainloader
