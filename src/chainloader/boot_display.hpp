#pragma once

#include "tft_display.hpp"

#include <stddef.h>
#include <stdint.h>

namespace Chainloader {

class BootDisplay {
public:
  explicit BootDisplay(TFTDisplay &display);

  void init();
  void showStage(const char *status, const char *detail = nullptr);
  void showTransferStart(uint32_t totalBytes);
  void updateTransferProgress(uint32_t receivedBytes, uint32_t totalBytes);
  void showError(const char *code, const char *detail = nullptr);

private:
  TFTDisplay &display_;
  bool initialized_;
  unsigned int statusAreaX_;
  unsigned int statusAreaY_;
  unsigned int statusAreaWidth_;
  unsigned int statusAreaHeight_;
  unsigned int detailAreaX_;
  unsigned int detailAreaY_;
  unsigned int detailAreaWidth_;
  unsigned int detailAreaHeight_;
  unsigned int progressBarX_;
  unsigned int progressBarY_;
  unsigned int progressBarWidth_;
  unsigned int progressBarHeight_;
  unsigned int progressFillWidth_;

  void drawStaticUi();
  void clearStatusArea();
  void clearDetailArea();
  void drawTextCentered(unsigned int y, const char *text, unsigned int scale,
                        uint16_t color);
  void drawText(unsigned int x, unsigned int y, const char *text,
                unsigned int scale, uint16_t color);
  void drawGlyph(unsigned int x, unsigned int y, char character,
                 unsigned int scale, uint16_t color);
  void drawLogoGlyph(unsigned int x, unsigned int y, char character,
                     unsigned int scale, uint16_t color);
  void drawCenteredLogo(unsigned int top, const char *logo, unsigned int scale,
                        uint16_t color);
  void setProgress(unsigned int percent);
  unsigned int glyphAdvance(unsigned int scale) const;
  unsigned int lineAdvance(unsigned int scale) const;
  unsigned int longestLineLength(const char *text) const;
  void formatBytes(char *buffer, size_t bufferSize, uint32_t bytes) const;
};

} // namespace Chainloader
