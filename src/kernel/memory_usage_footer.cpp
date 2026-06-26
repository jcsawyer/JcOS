#include "memory_usage_footer.hpp"

#include <common.hpp>
#include <memory/heap.hpp>

#include <stdio/printf.h>

namespace UI {
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

size_t stringLength(const char *text) {
  size_t length = 0;
  while (text != nullptr && text[length] != '\0') {
    ++length;
  }
  return length;
}

size_t findCharacter(const char *text, char value) {
  size_t index = 0;
  while (text != nullptr && text[index] != '\0') {
    if (text[index] == value) {
      return index;
    }
    ++index;
  }
  return index;
}

size_t findLastCharacter(const char *text, char value) {
  const size_t length = stringLength(text);
  for (size_t index = length; index > 0; --index) {
    if (text[index - 1] == value) {
      return index - 1;
    }
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

} // namespace

unsigned int StatusFooter::preferredHeight() const {
  return footerPaddingY * 2 + footerTextHeight + footerGap + footerBarHeight;
}

void StatusFooter::layout(const Size &displaySize) {
  const unsigned int footerHeight = preferredHeight();
  const unsigned int footerTop =
      displaySize.height > footerHeight ? displaySize.height - footerHeight : 0;

  footerBounds =
      Rect{0, static_cast<int>(footerTop), displaySize.width, footerHeight};
  textY = footerTop + footerPaddingY;
  barX = footerPaddingX;
  barY = footerTop + footerHeight - footerPaddingY - footerBarHeight;
  barWidth = displaySize.width > (footerPaddingX * 2)
                 ? displaySize.width - (footerPaddingX * 2)
                 : 0;
  pendingInvalidationCount = 0;
  queueInvalidation(footerBounds);
  initialized = false;
}

void StatusFooter::queueInvalidation(const Rect &region) {
  if (region.empty()) {
    return;
  }

  if (pendingInvalidationCount < maxPendingInvalidations) {
    pendingInvalidations[pendingInvalidationCount++] = region;
    return;
  }

  pendingInvalidations[maxPendingInvalidations - 1] = footerBounds;
}

void StatusFooter::formatUsageText(const Memory::HeapUsage &usage, char *buffer,
                                   size_t bufferCapacity) const {
  const unsigned long usedKiB =
      static_cast<unsigned long>(div_ceil(usage.usedBytes, KiB));
  const unsigned long totalKiB =
      static_cast<unsigned long>(div_ceil(usage.totalBytes, KiB));
  snprintf_(buffer, bufferCapacity, "Mem %05lu/%05lu KiB", usedKiB, totalKiB);
}

unsigned int
StatusFooter::computeFillWidth(const Memory::HeapUsage &usage) const {
  if (usage.totalBytes == 0 || barWidth == 0) {
    return 0;
  }

  size_t fillWidth = (usage.usedBytes * barWidth) / usage.totalBytes;
  if (fillWidth > barWidth) {
    fillWidth = barWidth;
  }

  return static_cast<unsigned int>(fillWidth);
}

void StatusFooter::measureText(const char *text, unsigned int *textXOut,
                               unsigned int *textWidthOut) const {
  const unsigned int textWidth =
      static_cast<unsigned int>(stringLength(text)) * footerTextAdvance;
  const unsigned int availableWidth = footerBounds.width;
  const unsigned int textX = availableWidth <= footerPaddingX ||
                                     textWidth + footerPaddingX > availableWidth
                                 ? 0
                                 : availableWidth - footerPaddingX - textWidth;

  *textXOut = textX;
  *textWidthOut = textWidth;
}

bool StatusFooter::tick(const Time::Duration &now) {
  (void)now;

  char currentText[footerTextCapacity];
  const Memory::HeapUsage usage = Memory::kernel_heap_usage();
  formatUsageText(usage, currentText, sizeof(currentText));
  const unsigned int currentFillWidth = computeFillWidth(usage);

  if (!initialized) {
    queueInvalidation(footerBounds);
    return true;
  }

  if (currentFillWidth != lastBarFillWidth) {
    const unsigned int startX = currentFillWidth < lastBarFillWidth
                                    ? currentFillWidth
                                    : lastBarFillWidth;
    const unsigned int width = currentFillWidth > lastBarFillWidth
                                   ? currentFillWidth - lastBarFillWidth
                                   : lastBarFillWidth - currentFillWidth;
    queueInvalidation(Rect{static_cast<int>(barX + startX),
                           static_cast<int>(barY), width, footerBarHeight});
  }

  if (!stringsEqual(lastText, currentText)) {
    unsigned int currentTextX = 0;
    unsigned int currentTextWidth = 0;
    measureText(currentText, &currentTextX, &currentTextWidth);

    const size_t lastSlashIndex = findCharacter(lastText, '/');
    const size_t currentSlashIndex = findCharacter(currentText, '/');
    const size_t sharedSlashIndex =
        lastSlashIndex < currentSlashIndex ? lastSlashIndex : currentSlashIndex;

    size_t usedFirstChangedIndex = 0;
    while (usedFirstChangedIndex < sharedSlashIndex &&
           lastText[usedFirstChangedIndex] == currentText[usedFirstChangedIndex]) {
      ++usedFirstChangedIndex;
    }

    const size_t usedFieldEndExclusive =
        (lastSlashIndex > currentSlashIndex ? lastSlashIndex : currentSlashIndex);
    if (usedFirstChangedIndex < usedFieldEndExclusive) {
      const unsigned int dirtyX =
          currentTextX +
          static_cast<unsigned int>(usedFirstChangedIndex) * footerTextAdvance;
      const unsigned int dirtyWidth =
          static_cast<unsigned int>(usedFieldEndExclusive - usedFirstChangedIndex) *
          footerTextAdvance;
      queueInvalidation(Rect{static_cast<int>(dirtyX), static_cast<int>(textY),
                             dirtyWidth, footerTextHeight});
    }

    const size_t lastSuffixIndex = findLastCharacter(lastText, ' ');
    const size_t currentSuffixIndex = findLastCharacter(currentText, ' ');
    const size_t totalFieldStart =
        sharedSlashIndex < stringLength(currentText) ? sharedSlashIndex + 1 : sharedSlashIndex;
    const size_t totalFieldEndExclusive =
        (lastSuffixIndex > currentSuffixIndex ? lastSuffixIndex : currentSuffixIndex);
    size_t totalFirstChangedIndex = totalFieldStart;
    while (totalFirstChangedIndex < lastSuffixIndex &&
           totalFirstChangedIndex < currentSuffixIndex &&
           lastText[totalFirstChangedIndex] == currentText[totalFirstChangedIndex]) {
      ++totalFirstChangedIndex;
    }

    if (totalFirstChangedIndex < totalFieldEndExclusive) {
      const unsigned int dirtyX =
          currentTextX +
          static_cast<unsigned int>(totalFirstChangedIndex) * footerTextAdvance;
      const unsigned int dirtyWidth =
          static_cast<unsigned int>(totalFieldEndExclusive - totalFirstChangedIndex) *
          footerTextAdvance;
      queueInvalidation(Rect{static_cast<int>(dirtyX), static_cast<int>(textY),
                             dirtyWidth, footerTextHeight});
    }
  }

  return pendingInvalidationCount > 0;
}

void StatusFooter::render(Surface &surface) {
  if (barWidth == 0 || footerBounds.empty()) {
    return;
  }

  const Memory::HeapUsage usage = Memory::kernel_heap_usage();
  char currentText[footerTextCapacity];
  formatUsageText(usage, currentText, sizeof(currentText));
  const unsigned int currentFillWidth = computeFillWidth(usage);

  surface.fillRect(footerBounds, footerBackgroundColor);
  surface.fillRect(Rect{static_cast<int>(barX), static_cast<int>(barY),
                        barWidth, footerBarHeight},
                   footerTrackColor);

  if (currentFillWidth > 0) {
    surface.fillRect(Rect{static_cast<int>(barX), static_cast<int>(barY),
                          currentFillWidth, footerBarHeight},
                     footerFillColor);
  }

  measureText(currentText, &lastTextX, &lastTextWidth);
  surface.drawString(
      Point{static_cast<int>(lastTextX), static_cast<int>(textY)}, currentText,
      footerTextColor, footerBackgroundColor, footerTextScale);

  copyString(lastText, currentText, sizeof(lastText));
  lastBarFillWidth = currentFillWidth;
  initialized = true;
}

Optional<Rect> StatusFooter::takeInvalidation() {
  if (pendingInvalidationCount == 0) {
    return Optional<Rect>();
  }

  Rect region = pendingInvalidations[0];
  for (size_t i = 1; i < pendingInvalidationCount; ++i) {
    pendingInvalidations[i - 1] = pendingInvalidations[i];
  }
  --pendingInvalidationCount;
  return Optional<Rect>(region);
}

} // namespace UI
