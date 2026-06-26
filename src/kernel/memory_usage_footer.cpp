#include "memory_usage_footer.hpp"

#include <bsp/raspberrypi/raspberrypi.hpp>
#include <common.hpp>
#include <display/display.hpp>
#include <memory/heap.hpp>
#include <task.hpp>
#include <time.hpp>
#include <time/duration.hpp>

#include <stdio/printf.h>

namespace UI {
namespace MemoryUsageFooter {
namespace {

constexpr uint16_t footerBackgroundColor = 0x0000;
constexpr uint16_t footerTextColor = 0xFFFF;
constexpr uint16_t footerTrackColor = 0x39E7;
constexpr uint16_t footerFillColor = 0x07E0;
constexpr unsigned int footerPaddingX = 4;
constexpr unsigned int footerPaddingY = 3;
constexpr unsigned int footerTextScale = 1;
constexpr unsigned int footerTextHeight = 7 * footerTextScale;
constexpr unsigned int footerTextAdvance = 6 * footerTextScale;
constexpr unsigned int footerBarHeight = 6;
constexpr unsigned int footerGap = 4;
constexpr unsigned int footerRefreshMillis = 250;
constexpr size_t footerTextCapacity = 32;

struct FooterLayout {
  unsigned int footerTop;
  unsigned int footerHeight;
  unsigned int textY;
  unsigned int barX;
  unsigned int barY;
  unsigned int barWidth;
};

struct FooterRenderState {
  bool initialized = false;
  unsigned int displayWidth = 0;
  unsigned int displayHeight = 0;
  unsigned int lastBarFillWidth = 0;
  unsigned int lastTextX = 0;
  unsigned int lastTextWidth = 0;
  char lastText[footerTextCapacity] = {};
};

FooterRenderState gFooterState;

size_t stringLength(const char *text) {
  size_t length = 0;
  while (text != nullptr && text[length] != '\0') {
    ++length;
  }
  return length;
}

bool stringsEqual(const char *lhs, const char *rhs) {
  size_t index = 0;
  while (lhs[index] != '\0' || rhs[index] != '\0') {
    if (lhs[index] != rhs[index]) {
      return false;
    }
    ++index;
  }
  return true;
}

void copyString(char *destination, const char *source, size_t capacity) {
  if (capacity == 0) {
    return;
  }

  size_t index = 0;
  for (; index + 1 < capacity && source[index] != '\0'; ++index) {
    destination[index] = source[index];
  }
  destination[index] = '\0';
}

FooterLayout computeLayout(Driver::Display::Display *display) {
  const unsigned int width = display->width();
  const unsigned int height = display->height();
  const unsigned int footerHeight =
      footerPaddingY * 2 + footerTextHeight + footerGap + footerBarHeight;
  const unsigned int footerTop =
      height > footerHeight ? height - footerHeight : 0;

  FooterLayout layout{};
  layout.footerTop = footerTop;
  layout.footerHeight = footerHeight;
  layout.textY = footerTop + footerPaddingY;
  layout.barX = footerPaddingX;
  layout.barY = footerTop + footerHeight - footerPaddingY - footerBarHeight;
  layout.barWidth =
      width > (footerPaddingX * 2) ? width - (footerPaddingX * 2) : 0;
  return layout;
}

void formatUsageText(const Memory::HeapUsage &usage, char *buffer,
                     size_t bufferCapacity) {
  const unsigned long usedKiB =
      static_cast<unsigned long>(div_ceil(usage.usedBytes, KiB));
  const unsigned long totalKiB =
      static_cast<unsigned long>(div_ceil(usage.totalBytes, KiB));
  snprintf_(buffer, bufferCapacity, "Mem %05lu/%05lu KiB", usedKiB, totalKiB);
}

unsigned int computeFillWidth(const Memory::HeapUsage &usage,
                              const FooterLayout &layout) {
  if (usage.totalBytes == 0 || layout.barWidth == 0) {
    return 0;
  }

  size_t fillWidth = (usage.usedBytes * layout.barWidth) / usage.totalBytes;
  if (fillWidth > layout.barWidth) {
    fillWidth = layout.barWidth;
  }

  return static_cast<unsigned int>(fillWidth);
}

void measureText(Driver::Display::Display *display, const char *text,
                 unsigned int *textXOut, unsigned int *textWidthOut) {
  const unsigned int textWidth =
      static_cast<unsigned int>(stringLength(text)) * footerTextAdvance;
  const unsigned int availableWidth = display->width();
  const unsigned int textX = availableWidth <= footerPaddingX ||
                                     textWidth + footerPaddingX > availableWidth
                                 ? 0
                                 : availableWidth - footerPaddingX - textWidth;

  *textXOut = textX;
  *textWidthOut = textWidth;
}

void drawText(Driver::Display::Display *display, const FooterLayout &layout,
              const char *text, unsigned int textX) {
  display->drawString(textX, layout.textY, text, footerTextColor,
                      footerBackgroundColor, footerTextScale);
}

void redrawChangedCharacters(Driver::Display::Display *display,
                             const FooterLayout &layout,
                             const char *previousText, const char *currentText,
                             unsigned int textX) {
  const size_t previousLength = stringLength(previousText);
  const size_t currentLength = stringLength(currentText);
  const size_t commonLength =
      previousLength < currentLength ? previousLength : currentLength;

  for (size_t i = 0; i < commonLength; i++) {
    if (previousText[i] == currentText[i]) {
      continue;
    }

    const unsigned int glyphX =
        textX + static_cast<unsigned int>(i) * footerTextAdvance;
    display->drawMonoGlyph(glyphX, layout.textY, currentText[i],
                           footerTextColor, footerBackgroundColor,
                           footerTextScale);
  }
}

void redrawFooter(Driver::Display::Display *display,
                  const Memory::HeapUsage &usage) {
  FooterLayout layout = computeLayout(display);
  if (layout.barWidth == 0) {
    return;
  }

  char currentText[footerTextCapacity];
  formatUsageText(usage, currentText, sizeof(currentText));
  const unsigned int currentFillWidth = computeFillWidth(usage, layout);

  if (!gFooterState.initialized ||
      gFooterState.displayWidth != display->width() ||
      gFooterState.displayHeight != display->height()) {
    display->fillRect(0, layout.footerTop, display->width(),
                      layout.footerHeight, footerBackgroundColor);
    display->fillRect(layout.barX, layout.barY, layout.barWidth,
                      footerBarHeight, footerTrackColor);

    if (currentFillWidth > 0) {
      display->fillRect(layout.barX, layout.barY, currentFillWidth,
                        footerBarHeight, footerFillColor);
    }

    measureText(display, currentText, &gFooterState.lastTextX,
                &gFooterState.lastTextWidth);
    drawText(display, layout, currentText, gFooterState.lastTextX);
    copyString(gFooterState.lastText, currentText,
               sizeof(gFooterState.lastText));
    gFooterState.lastBarFillWidth = currentFillWidth;
    gFooterState.displayWidth = display->width();
    gFooterState.displayHeight = display->height();
    gFooterState.initialized = true;
    return;
  }

  if (!stringsEqual(gFooterState.lastText, currentText)) {
    unsigned int currentTextX = 0;
    unsigned int currentTextWidth = 0;
    measureText(display, currentText, &currentTextX, &currentTextWidth);

    if (gFooterState.lastTextX == currentTextX &&
        gFooterState.lastTextWidth == currentTextWidth) {
      redrawChangedCharacters(display, layout, gFooterState.lastText,
                              currentText, currentTextX);
    } else {
      unsigned int clearX = gFooterState.lastTextX < currentTextX
                                ? gFooterState.lastTextX
                                : currentTextX;
      unsigned int clearEnd =
          (gFooterState.lastTextX + gFooterState.lastTextWidth) >
                  (currentTextX + currentTextWidth)
              ? (gFooterState.lastTextX + gFooterState.lastTextWidth)
              : (currentTextX + currentTextWidth);

      if (clearEnd > clearX) {
        display->fillRect(clearX, layout.textY, clearEnd - clearX,
                          footerTextHeight, footerBackgroundColor);
        drawText(display, layout, currentText, currentTextX);
      }
    }

    gFooterState.lastTextX = currentTextX;
    gFooterState.lastTextWidth = currentTextWidth;
    copyString(gFooterState.lastText, currentText,
               sizeof(gFooterState.lastText));
  }

  if (currentFillWidth > gFooterState.lastBarFillWidth) {
    display->fillRect(layout.barX + gFooterState.lastBarFillWidth, layout.barY,
                      currentFillWidth - gFooterState.lastBarFillWidth,
                      footerBarHeight, footerFillColor);
  } else if (currentFillWidth < gFooterState.lastBarFillWidth) {
    display->fillRect(layout.barX + currentFillWidth, layout.barY,
                      gFooterState.lastBarFillWidth - currentFillWidth,
                      footerBarHeight, footerTrackColor);
  }

  gFooterState.lastBarFillWidth = currentFillWidth;
}

void taskEntry() {
  Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
  Time::Duration nextUpdate = Time::Duration::zero();

  while (true) {
    Tasks::yield();

    if (timeManager->uptime() < nextUpdate) {
      continue;
    }

    nextUpdate = timeManager->uptime() +
                 Time::Duration::from_millis(footerRefreshMillis);

    Driver::Display::Display *display =
        Driver::BSP::RaspberryPi::RaspberryPi::getDisplay();
    if (display == nullptr || !display->isReady()) {
      continue;
    }

    redrawFooter(display, Memory::kernel_heap_usage());
  }
}

} // namespace

void start() { taskManager.addTask("memory-footer", taskEntry); }

} // namespace MemoryUsageFooter
} // namespace UI
