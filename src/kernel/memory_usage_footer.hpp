#pragma once

#include <memory/heap.hpp>
#include <time/duration.hpp>
#include <ui.hpp>

namespace UI {

class StatusFooter {
public:
  unsigned int preferredHeight() const;
  Rect bounds() const { return footerBounds; }

  void layout(const Size &displaySize);
  bool tick(const Time::Duration &now);
  void render(Surface &surface);
  Optional<Rect> takeInvalidation();

private:
  void formatUsageText(const Memory::HeapUsage &usage, char *buffer,
                       size_t bufferCapacity) const;
  unsigned int computeFillWidth(const Memory::HeapUsage &usage) const;
  void measureText(const char *text, unsigned int *textXOut,
                   unsigned int *textWidthOut) const;
  void queueInvalidation(const Rect &region);

  static constexpr size_t footerTextCapacity = 32;
  static constexpr size_t maxPendingInvalidations = 4;

  Rect footerBounds{};
  Rect pendingInvalidations[maxPendingInvalidations] = {};
  size_t pendingInvalidationCount = 0;
  unsigned int textY = 0;
  unsigned int barX = 0;
  unsigned int barY = 0;
  unsigned int barWidth = 0;
  unsigned int lastBarFillWidth = 0;
  unsigned int lastTextX = 0;
  unsigned int lastTextWidth = 0;
  char lastText[footerTextCapacity] = {};
  bool initialized = false;
};

} // namespace UI
