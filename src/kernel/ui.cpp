#include "ui.hpp"

#include <common.hpp>
#include <memory_usage_footer.hpp>
#include <panic.hpp>
#include <print.hpp>
#include <task.hpp>
#include <time.hpp>

namespace UI {
namespace {

constexpr uint16_t backgroundColor = 0x0000;
constexpr uint16_t textColor = 0xFFFF;
constexpr uint16_t accentColor = 0x07FF;
constexpr uint16_t mutedColor = 0x7BEF;
constexpr uint16_t touchIndicatorColor = 0xFBE0;
const Time::Duration uiTickInterval = Time::Duration::from_millis(16);
const Time::Duration minRenderInterval = Time::Duration::from_millis(4);
const Time::Duration tapMaxDuration = Time::Duration::from_millis(250);
const Time::Duration touchIdlePollInterval = Time::Duration::from_millis(8);
const Time::Duration touchActivePollInterval = Time::Duration::zero();
const Time::Duration touchReleaseTimeout = Time::Duration::from_millis(24);
constexpr unsigned int pointerMoveThreshold = 1;
constexpr unsigned int tapMoveThreshold = 12;
constexpr int touchIndicatorRadius = 8;
constexpr unsigned int touchIndicatorThickness = 2;
constexpr unsigned int touchIndicatorSnapStep = 6;
constexpr size_t maxOverlayInvalidations = 4;
constexpr size_t maxInputEventsPerSourcePerPump = 8;

const char *eventTypeName(InputEventType type) {
  switch (type) {
  case InputEventType::NavigateUp:
    return "Nav Up";
  case InputEventType::NavigateDown:
    return "Nav Down";
  case InputEventType::NavigateLeft:
    return "Nav Left";
  case InputEventType::NavigateRight:
    return "Nav Right";
  case InputEventType::Select:
    return "Select";
  case InputEventType::Back:
    return "Back";
  case InputEventType::PointerDown:
    return "Touch Down";
  case InputEventType::PointerMove:
    return "Touch Move";
  case InputEventType::PointerUp:
    return "Touch Up";
  case InputEventType::Tick:
    return "Tick";
  }

  return "Unknown";
}

void writeDecimal(unsigned long value, char *buffer, size_t bufferCapacity) {
  if (bufferCapacity == 0) {
    return;
  }

  size_t index = 0;
  if (value == 0) {
    if (bufferCapacity > 1) {
      buffer[index++] = '0';
    }
  } else {
    char reversed[24];
    size_t reversedCount = 0;
    while (value > 0 && reversedCount < sizeof(reversed)) {
      reversed[reversedCount++] = static_cast<char>('0' + (value % 10));
      value /= 10;
    }

    while (reversedCount > 0 && index + 1 < bufferCapacity) {
      buffer[index++] = reversed[--reversedCount];
    }
  }

  buffer[index] = '\0';
}

size_t stringLength(const char *text) {
  size_t length = 0;
  while (text != nullptr && text[length] != '\0') {
    ++length;
  }
  return length;
}

Rect unionRect(const Rect &lhs, const Rect &rhs) {
  if (lhs.empty()) {
    return rhs;
  }
  if (rhs.empty()) {
    return lhs;
  }

  const int x0 = lhs.x < rhs.x ? lhs.x : rhs.x;
  const int y0 = lhs.y < rhs.y ? lhs.y : rhs.y;
  const int x1 = lhs.right() > rhs.right() ? lhs.right() : rhs.right();
  const int y1 = lhs.bottom() > rhs.bottom() ? lhs.bottom() : rhs.bottom();
  return Rect{x0, y0, static_cast<unsigned int>(x1 - x0),
              static_cast<unsigned int>(y1 - y0)};
}

Point snapPointToIndicatorGrid(const Point &point) {
  const int step = static_cast<int>(touchIndicatorSnapStep);
  return Point{(point.x / step) * step, (point.y / step) * step};
}

class TouchIndicatorOverlay : public Overlay {
public:
  explicit TouchIndicatorOverlay(Runtime *runtime) : runtime(runtime) {}

  void layout(const Rect &bounds) override {
    frame = bounds;
    pendingInvalidationCount = 0;
    lastRenderedVisible = false;
    lastRenderedBounds = Rect{};
  }

  void render(Surface &surface) override {
    if (runtime == nullptr) {
      return;
    }

    const UiSettings &settings = runtime->settings();
    const PointerPresentation &pointer = runtime->pointerState();
    if (!settings.showTouchIndicator || !pointer.visible) {
      lastRenderedVisible = false;
      lastRenderedBounds = Rect{};
      return;
    }

    const Rect markerBounds = indicatorBounds(pointer.position);
    if (markerBounds.empty()) {
      lastRenderedVisible = false;
      lastRenderedBounds = Rect{};
      return;
    }

    const int outerDiameter = touchIndicatorRadius * 2 + 1;

    surface.fillRect(
        Rect{pointer.position.x - touchIndicatorRadius,
             pointer.position.y - static_cast<int>(touchIndicatorThickness / 2),
             outerDiameter, touchIndicatorThickness},
        touchIndicatorColor);
    surface.fillRect(
        Rect{pointer.position.x - static_cast<int>(touchIndicatorThickness / 2),
             pointer.position.y - touchIndicatorRadius, touchIndicatorThickness,
             outerDiameter},
        touchIndicatorColor);

    lastRenderedVisible = true;
    lastRenderedBounds = markerBounds;
  }

  Optional<Rect> takeInvalidation() override {
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

  void syncTransition(const PointerPresentation &previous,
                      const PointerPresentation &current,
                      bool indicatorEnabled) {
    const Rect previousBounds =
        indicatorEnabled && previous.visible ? indicatorBounds(previous.position)
                                            : Rect{};
    const Rect currentBounds =
        indicatorEnabled && current.visible ? indicatorBounds(current.position)
                                            : Rect{};
    queueInvalidation(unionRect(previousBounds, currentBounds));
  }

private:
  Rect indicatorBounds(const Point &center) const {
    const int diameter = touchIndicatorRadius * 2 + 1;
    return clippedToFrame(Rect{center.x - touchIndicatorRadius,
                               center.y - touchIndicatorRadius, diameter,
                               diameter});
  }

  Rect clippedToFrame(const Rect &rect) const {
    const int x0 = rect.x > frame.x ? rect.x : frame.x;
    const int y0 = rect.y > frame.y ? rect.y : frame.y;
    const int x1 = rect.right() < frame.right() ? rect.right() : frame.right();
    const int y1 =
        rect.bottom() < frame.bottom() ? rect.bottom() : frame.bottom();

    if (x1 <= x0 || y1 <= y0) {
      return Rect{};
    }

    return Rect{x0, y0, static_cast<unsigned int>(x1 - x0),
                static_cast<unsigned int>(y1 - y0)};
  }

  void queueInvalidation(const Rect &region) {
    if (region.empty()) {
      return;
    }

    if (pendingInvalidationCount < maxOverlayInvalidations) {
      pendingInvalidations[pendingInvalidationCount++] = region;
      return;
    }

    pendingInvalidations[maxOverlayInvalidations - 1] =
        unionRect(pendingInvalidations[maxOverlayInvalidations - 1], region);
  }

  Runtime *runtime;
  Rect frame{};
  Rect pendingInvalidations[maxOverlayInvalidations] = {};
  size_t pendingInvalidationCount = 0;
  bool lastRenderedVisible = false;
  Rect lastRenderedBounds{};
};

class HomeScreen : public View {
public:
  void layout(const Rect &bounds) override {
    frame = bounds;
    pendingInvalidation = frame;
    hasPendingInvalidation = true;
  }

  void render(Surface &surface) override {
    surface.drawString(Point{frame.x + 18, frame.y + 18}, "JcOS", accentColor,
                       backgroundColor, 3);
    surface.drawString(Point{frame.x + 18, frame.y + 50}, "Kernel UI shell",
                       textColor, backgroundColor, 1);
    surface.drawString(Point{frame.x + 18, frame.y + 76}, "Version 0.1.0",
                       mutedColor, backgroundColor, 1);

    renderLogo(surface);
    renderStats(surface);
  }

  bool handleInput(const InputEvent &event) override {
    if (event.type == InputEventType::PointerMove) {
      return false;
    }

    lastEvent = event.type;
    ++eventCount;
    markStatsDirty();
    return true;
  }

  bool tick(const Time::Duration &now) override {
    const unsigned long uptimeSeconds =
        static_cast<unsigned long>(now.as_secs());
    if (!initialized) {
      initialized = true;
      previousUptimeSeconds = uptimeSeconds;
      lastUptimeSeconds = uptimeSeconds;
      markStatsDirty();
      return true;
    }

    if (uptimeSeconds != lastUptimeSeconds) {
      previousUptimeSeconds = lastUptimeSeconds;
      lastUptimeSeconds = uptimeSeconds;
      markUptimeDirty();
      return true;
    }

    return false;
  }

  Optional<Rect> takeInvalidation() override {
    if (!hasPendingInvalidation) {
      return Optional<Rect>();
    }

    Rect region = pendingInvalidation;
    hasPendingInvalidation = false;
    return Optional<Rect>(region);
  }

private:
  void mergeInvalidation(const Rect &region) {
    if (region.empty()) {
      return;
    }

    if (!hasPendingInvalidation) {
      pendingInvalidation = region;
      hasPendingInvalidation = true;
      return;
    }

    pendingInvalidation = unionRect(pendingInvalidation, region);
  }

  void markUptimeDirty() {
    const int statsY = frame.y + 210;
    char previousUptimeBuffer[24];
    char currentUptimeBuffer[24];
    writeDecimal(previousUptimeSeconds, previousUptimeBuffer,
                 sizeof(previousUptimeBuffer));
    writeDecimal(lastUptimeSeconds, currentUptimeBuffer,
                 sizeof(currentUptimeBuffer));

    size_t firstChangedIndex = 0;
    while (previousUptimeBuffer[firstChangedIndex] != '\0' &&
           currentUptimeBuffer[firstChangedIndex] != '\0' &&
           previousUptimeBuffer[firstChangedIndex] ==
               currentUptimeBuffer[firstChangedIndex]) {
      ++firstChangedIndex;
    }

    const size_t previousLength = stringLength(previousUptimeBuffer);
    const size_t currentLength = stringLength(currentUptimeBuffer);
    const size_t maxLength =
        previousLength > currentLength ? previousLength : currentLength;
    if (firstChangedIndex < maxLength) {
      mergeInvalidation(Rect{
          frame.x + 220 + static_cast<int>(firstChangedIndex * 6), statsY + 36,
          static_cast<unsigned int>(maxLength - firstChangedIndex) * 6, 7});
    }
  }

  void markStatsDirty() {
    const int statsY = frame.y + 210;
    mergeInvalidation(Rect{frame.x + 214, statsY + 52, 128, 34});
  }

  void renderLogo(Surface &surface) {
    static const char *logo[] = {
        "    __",
        "   |  |    _____ _____",
        "   |  |___|     |   __|",
        " __|  |  _|  |  |__   |",
        "|_____|___|_____|_____|",
    };

    const Point origin{frame.x + 18, frame.y + 110};
    for (size_t i = 0; i < (sizeof(logo) / sizeof(logo[0])); ++i) {
      surface.drawString(Point{origin.x, origin.y + static_cast<int>(i * 16)},
                         logo[i], textColor, backgroundColor, 2);
    }
  }

  void renderStats(Surface &surface) {
    const int statsY = frame.y + 210;
    const int left = frame.x + 18;
    const int right = frame.x + 220;

    char uptimeBuffer[24];
    writeDecimal(lastUptimeSeconds, uptimeBuffer, sizeof(uptimeBuffer));

    char widthBuffer[16];
    writeDecimal(frame.width, widthBuffer, sizeof(widthBuffer));

    char heightBuffer[16];
    writeDecimal(frame.height, heightBuffer, sizeof(heightBuffer));

    char eventCountBuffer[24];
    writeDecimal(eventCount, eventCountBuffer, sizeof(eventCountBuffer));

    surface.drawString(Point{left, statsY}, "Mode", mutedColor, backgroundColor,
                       1);
    surface.drawString(Point{right, statsY}, "App Shell", textColor,
                       backgroundColor, 1);

    surface.drawString(Point{left, statsY + 18}, "Display", mutedColor,
                       backgroundColor, 1);
    surface.drawString(Point{right, statsY + 18}, widthBuffer, textColor,
                       backgroundColor, 1);
    surface.drawString(Point{right + 36, statsY + 18}, "x", textColor,
                       backgroundColor, 1);
    surface.drawString(Point{right + 48, statsY + 18}, heightBuffer, textColor,
                       backgroundColor, 1);

    surface.drawString(Point{left, statsY + 36}, "Uptime", mutedColor,
                       backgroundColor, 1);
    surface.drawString(Point{right, statsY + 36}, uptimeBuffer, textColor,
                       backgroundColor, 1);
    surface.drawString(Point{right + 42, statsY + 36}, "sec", textColor,
                       backgroundColor, 1);

    surface.drawString(Point{left, statsY + 54}, "Input", mutedColor,
                       backgroundColor, 1);
    surface.drawString(Point{right, statsY + 54}, eventTypeName(lastEvent),
                       textColor, backgroundColor, 1);

    surface.drawString(Point{left, statsY + 72}, "Events", mutedColor,
                       backgroundColor, 1);
    surface.drawString(Point{right, statsY + 72}, eventCountBuffer, textColor,
                       backgroundColor, 1);
  }

  Rect frame{};
  Rect pendingInvalidation{};
  bool hasPendingInvalidation = false;
  bool initialized = false;
  unsigned long previousUptimeSeconds = 0;
  unsigned long lastUptimeSeconds = 0;
  unsigned long eventCount = 0;
  InputEventType lastEvent = InputEventType::Tick;
};

} // namespace

Surface::Surface(Driver::Display::Display *display)
    : display(display), clipRect(bounds()) {}

TouchInputSource::TouchInputSource(Driver::BSP::Touch::TouchPanel *touchPanel,
                                   Driver::Display::Display *display)
    : touchPanel(touchPanel), display(display) {}

Optional<InputEvent> TouchInputSource::pollEvent() {
  const Time::Duration now = Time::TimeManager::GetInstance()->uptime();

  if (selectPending) {
    selectPending = false;
    InputEvent selectEvent{};
    selectEvent.type = InputEventType::Select;
    selectEvent.position = releasePoint;
    selectEvent.timestamp = now;
    return Optional<InputEvent>(selectEvent);
  }

  if (touchPanel == nullptr || display == nullptr || !touchPanel->isReady()) {
    return Optional<InputEvent>();
  }

  const bool shouldPollActiveTouch =
      pointerActive && (now - lastTouchPollAt) >= touchActivePollInterval;
  const bool shouldPollIdleTouch =
      !pointerActive && (now - lastTouchPollAt) >= touchIdlePollInterval;
  const bool hasPendingTouchSample = touchPanel->hasPendingSample();

  if (!hasPendingTouchSample && !shouldPollActiveTouch && !shouldPollIdleTouch) {
    if (pointerActive && (now - lastTouchSampleAt) >= touchReleaseTimeout) {
      pointerActive = false;
      releasePoint = lastPoint;

      InputEvent releaseEvent{};
      releaseEvent.type = InputEventType::PointerUp;
      releaseEvent.position = releasePoint;
      releaseEvent.timestamp = now;

      if (tapEligible && (now - pressStartedAt) <= tapMaxDuration) {
        selectPending = true;
      }
      tapEligible = false;

      return Optional<InputEvent>(releaseEvent);
    }

    return Optional<InputEvent>();
  }

  Optional<Driver::BSP::Touch::Sample> sample = touchPanel->readSample();
  lastTouchPollAt = now;
  if (!sample.has_value()) {
    return Optional<InputEvent>();
  }

  const Point mappedPoint = mapToDisplay(sample.value());
  const Point snappedPoint = snapPointToIndicatorGrid(mappedPoint);
  InputEvent event{};
  event.position = snappedPoint;
  event.timestamp = now;
  lastTouchSampleAt = now;

  if (sample.value().active) {
    if (!pointerActive) {
      pointerActive = true;
      tapEligible = true;
      pressPoint = snappedPoint;
      lastPoint = snappedPoint;
      pressStartedAt = now;
      event.type = InputEventType::PointerDown;
      return Optional<InputEvent>(event);
    }

    if (movedFarEnough(lastPoint, snappedPoint, pointerMoveThreshold)) {
      if (movedFarEnough(pressPoint, mappedPoint, tapMoveThreshold)) {
        tapEligible = false;
      }
      lastPoint = snappedPoint;
      event.type = InputEventType::PointerMove;
      return Optional<InputEvent>(event);
    }

    if (movedFarEnough(pressPoint, mappedPoint, tapMoveThreshold)) {
      tapEligible = false;
    }

    return Optional<InputEvent>();
  }

  if (!pointerActive) {
    return Optional<InputEvent>();
  }

  pointerActive = false;
  releasePoint = lastPoint;
  event.type = InputEventType::PointerUp;
  event.position = releasePoint;

  if (tapEligible && (now - pressStartedAt) <= tapMaxDuration) {
    selectPending = true;
  }
  tapEligible = false;

  return Optional<InputEvent>(event);
}

Point TouchInputSource::mapToDisplay(
    const Driver::BSP::Touch::Sample &sample) const {
  const unsigned int rawWidth = touchPanel->rawWidth();
  const unsigned int rawHeight = touchPanel->rawHeight();

  unsigned int boundedX = sample.x < rawWidth ? sample.x : rawWidth - 1;
  unsigned int boundedY = sample.y < rawHeight ? sample.y : rawHeight - 1;

  switch (display->rotation()) {
  case Driver::Display::Display::Rotation::Portrait:
    return Point{static_cast<int>(boundedX), static_cast<int>(boundedY)};
  case Driver::Display::Display::Rotation::Landscape:
    return Point{static_cast<int>(boundedY),
                 static_cast<int>(rawWidth - 1 - boundedX)};
  case Driver::Display::Display::Rotation::InvertedPortrait:
    return Point{static_cast<int>(rawWidth - 1 - boundedX),
                 static_cast<int>(rawHeight - 1 - boundedY)};
  case Driver::Display::Display::Rotation::InvertedLandscape:
    return Point{static_cast<int>(rawHeight - 1 - boundedY),
                 static_cast<int>(boundedX)};
  }

  return Point{};
}

bool TouchInputSource::movedFarEnough(const Point &lhs, const Point &rhs,
                                      unsigned int threshold) {
  const int deltaX = lhs.x > rhs.x ? lhs.x - rhs.x : rhs.x - lhs.x;
  const int deltaY = lhs.y > rhs.y ? lhs.y - rhs.y : rhs.y - lhs.y;
  return static_cast<unsigned int>(deltaX) >= threshold ||
         static_cast<unsigned int>(deltaY) >= threshold;
}

Size Surface::size() const {
  if (display == nullptr) {
    return Size{};
  }

  return Size{display->width(), display->height()};
}

Rect Surface::bounds() const {
  const Size surfaceSize = size();
  return Rect{0, 0, surfaceSize.width, surfaceSize.height};
}

Rect Surface::clipped(const Rect &rect) const {
  Rect bounded = bounds();
  const int x0 = rect.x > clipRect.x ? rect.x : clipRect.x;
  const int y0 = rect.y > clipRect.y ? rect.y : clipRect.y;
  const int x1 =
      rect.right() < clipRect.right() ? rect.right() : clipRect.right();
  const int y1 =
      rect.bottom() < clipRect.bottom() ? rect.bottom() : clipRect.bottom();

  const int bx0 = x0 > bounded.x ? x0 : bounded.x;
  const int by0 = y0 > bounded.y ? y0 : bounded.y;
  const int bx1 = x1 < bounded.right() ? x1 : bounded.right();
  const int by1 = y1 < bounded.bottom() ? y1 : bounded.bottom();

  if (bx1 <= bx0 || by1 <= by0) {
    return Rect{};
  }

  return Rect{bx0, by0, static_cast<unsigned int>(bx1 - bx0),
              static_cast<unsigned int>(by1 - by0)};
}

void Surface::setClipRect(const Rect &rect) { clipRect = clipped(rect); }

void Surface::resetClipRect() { clipRect = bounds(); }

void Surface::clear(uint16_t color) {
  if (display == nullptr) {
    return;
  }

  resetClipRect();
  display->clear(color);
}

void Surface::fillRect(const Rect &rect, uint16_t color) {
  if (display == nullptr) {
    return;
  }

  Rect area = clipped(rect);
  if (area.empty()) {
    return;
  }

  display->fillRect(static_cast<unsigned int>(area.x),
                    static_cast<unsigned int>(area.y), area.width, area.height,
                    color);
}

void Surface::drawMonoGlyph(const Point &origin, char value,
                            uint16_t foreground, uint16_t background,
                            unsigned int scale) {
  const Driver::Display::Glyph5x7 &glyph =
      Driver::Display::lookupGlyph5x7(value);

  const unsigned int glyphHeight = 7 * scale;
  const unsigned int glyphAdvance = 6 * scale;

  fillRect(Rect{origin.x, origin.y, glyphAdvance, glyphHeight}, background);

  for (unsigned int row = 0; row < 7; ++row) {
    unsigned int col = 0;
    while (col < 5) {
      while (col < 5 && (glyph.rows[row] & (1u << (4 - col))) == 0) {
        ++col;
      }

      const unsigned int runStart = col;
      while (col < 5 && (glyph.rows[row] & (1u << (4 - col))) != 0) {
        ++col;
      }

      if (runStart < col) {
        fillRect(Rect{origin.x + static_cast<int>(runStart * scale),
                      origin.y + static_cast<int>(row * scale),
                      (col - runStart) * scale, scale},
                 foreground);
      }
    }
  }
}

void Surface::drawString(const Point &origin, const char *text,
                         uint16_t foreground, uint16_t background,
                         unsigned int scale) {
  if (text == nullptr) {
    return;
  }

  const unsigned int glyphAdvance = 6 * scale;
  int cursorX = origin.x;
  int cursorY = origin.y;

  while (*text != '\0') {
    if (*text == '\n') {
      cursorX = origin.x;
      cursorY += static_cast<int>(8 * scale);
    } else {
      drawMonoGlyph(Point{cursorX, cursorY}, *text, foreground, background,
                    scale);
      cursorX += static_cast<int>(glyphAdvance);
    }
    ++text;
  }
}

void ScreenManager::setRootScreen(View *view) {
  if (view == nullptr) {
    panic("UI root screen cannot be null");
  }

  screens[0] = view;
  screenCount = 1;
}

bool ScreenManager::pushScreen(View *view) {
  if (view == nullptr || screenCount >= maxScreens) {
    return false;
  }

  screens[screenCount++] = view;
  return true;
}

void ScreenManager::popScreen() {
  if (screenCount > 1) {
    screens[--screenCount] = nullptr;
  }
}

bool ScreenManager::pushOverlay(Overlay *overlay) {
  if (overlay == nullptr || overlayCount >= maxOverlays) {
    return false;
  }

  overlays[overlayCount++] = overlay;
  return true;
}

void ScreenManager::popOverlay() {
  if (overlayCount > 0) {
    overlays[--overlayCount] = nullptr;
  }
}

View *ScreenManager::activeScreen() const {
  if (screenCount == 0) {
    return nullptr;
  }

  return screens[screenCount - 1];
}

Overlay *ScreenManager::activeOverlay() const {
  if (overlayCount == 0) {
    return nullptr;
  }

  return overlays[overlayCount - 1];
}

void ScreenManager::layout(const Rect &screenBounds,
                           const Rect &overlayBounds) {
  for (size_t i = 0; i < screenCount; ++i) {
    if (screens[i] != nullptr) {
      screens[i]->layout(screenBounds);
    }
  }

  for (size_t i = 0; i < overlayCount; ++i) {
    if (overlays[i] != nullptr) {
      overlays[i]->layout(overlayBounds);
    }
  }
}

bool ScreenManager::handleInput(const InputEvent &event) {
  if (overlayCount > 0 && overlays[overlayCount - 1] != nullptr &&
      overlays[overlayCount - 1]->handleInput(event)) {
    return true;
  }

  View *screen = activeScreen();
  return screen != nullptr ? screen->handleInput(event) : false;
}

bool ScreenManager::tick(const Time::Duration &now) {
  bool changed = false;

  for (size_t i = 0; i < screenCount; ++i) {
    if (screens[i] != nullptr && screens[i]->tick(now)) {
      changed = true;
    }
  }

  for (size_t i = 0; i < overlayCount; ++i) {
    if (overlays[i] != nullptr && overlays[i]->tick(now)) {
      changed = true;
    }
  }

  return changed;
}

void Renderer::render(Surface &surface, ScreenManager &screenManager,
                      StatusFooter *footer) {
  surface.clear(backgroundColor);

  View *activeScreen = screenManager.activeScreen();
  if (activeScreen != nullptr) {
    activeScreen->render(surface);
  }

  if (footer != nullptr) {
    footer->render(surface);
  }

  Overlay *activeOverlay = screenManager.activeOverlay();
  if (activeOverlay != nullptr) {
    activeOverlay->render(surface);
  }
}

namespace {
void renderDirtyRegion(Surface &surface, ScreenManager &screenManager,
                       StatusFooter *footer, const Rect &region) {
  if (region.empty()) {
    return;
  }

  surface.setClipRect(region);
  surface.fillRect(region, backgroundColor);

  View *activeScreen = screenManager.activeScreen();
  if (activeScreen != nullptr) {
    activeScreen->render(surface);
  }

  if (footer != nullptr) {
    footer->render(surface);
  }

  Overlay *activeOverlay = screenManager.activeOverlay();
  if (activeOverlay != nullptr) {
    activeOverlay->render(surface);
  }

  surface.resetClipRect();
}
} // namespace

bool Runtime::EventQueue::push(const InputEvent &event) {
  if (count >= maxInputEvents) {
    return false;
  }

  events[writeIndex] = event;
  writeIndex = (writeIndex + 1) % maxInputEvents;
  ++count;
  return true;
}

Optional<InputEvent> Runtime::EventQueue::pop() {
  if (count == 0) {
    return Optional<InputEvent>();
  }

  InputEvent event = events[readIndex];
  readIndex = (readIndex + 1) % maxInputEvents;
  --count;
  return Optional<InputEvent>(event);
}

Runtime *Runtime::GetInstance() {
  static Runtime instance;
  return &instance;
}

void Runtime::init(Driver::Display::Display *newDisplay) {
  display = newDisplay;
  if (display == nullptr || !display->isReady()) {
    warn("UI runtime unavailable; display not ready");
    return;
  }

  surface = new Surface(display);
  homeScreen = new HomeScreen();
  footer = new StatusFooter();
  touchIndicatorOverlay = new TouchIndicatorOverlay(this);

  screenManager.setRootScreen(homeScreen);
  screenManager.pushOverlay(touchIndicatorOverlay);
  footer->layout(surface->size());

  Rect overlayBounds = surface->bounds();
  Rect screenBounds = overlayBounds;
  const unsigned int footerHeight = footer->preferredHeight();
  if (screenBounds.height > footerHeight) {
    screenBounds.height -= footerHeight;
  }

  screenManager.layout(screenBounds, overlayBounds);
  lastTickAt = Time::TimeManager::GetInstance()->uptime();
  lastRenderAt = Time::Duration::zero();
  ready = true;
  fullRedrawPending = true;
}

void Runtime::start() {
  if (!ready || started) {
    return;
  }

  started = true;
  taskManager.addTask("ui-runtime", &Runtime::taskEntry);
}

void Runtime::invalidate(const Rect &rect) {
  if (!ready) {
    return;
  }

  if (dirtyRectCount < maxDirtyRects) {
    dirtyRects[dirtyRectCount++] = rect;
  } else {
    fullRedrawPending = true;
  }
}

void Runtime::invalidateAll() {
  if (!ready) {
    return;
  }

  fullRedrawPending = true;
  dirtyRectCount = 0;
}

bool Runtime::postInputEvent(const InputEvent &event) {
  return ready ? eventQueue.push(event) : false;
}

bool Runtime::registerInputSource(InputSource *inputSource) {
  if (!ready || inputSource == nullptr || inputSourceCount >= maxInputSources) {
    return false;
  }

  inputSources[inputSourceCount++] = inputSource;
  return true;
}

void Runtime::taskEntry() {
  Runtime *runtime = Runtime::GetInstance();
  while (true) {
    runtime->pump();
    Tasks::yield();
  }
}

void Runtime::pump() {
  if (!ready) {
    return;
  }

  Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
  const Time::Duration now = timeManager->uptime();

  pollInputSources();

  bool stateChanged = false;
  stateChanged = dispatchEvents() || stateChanged;

  if (now >= (lastTickAt + uiTickInterval)) {
    stateChanged = advanceUiState(now) || stateChanged;
    lastTickAt = now;
  }

  const bool hadDirtyBeforeDrain = fullRedrawPending || dirtyRectCount > 0;
  const bool drainedInvalidations = drainViewInvalidations();

  if (stateChanged && !hadDirtyBeforeDrain && !drainedInvalidations &&
      !fullRedrawPending && dirtyRectCount == 0) {
    invalidateAll();
  }

  if ((fullRedrawPending || dirtyRectCount > 0) &&
      now >= (lastRenderAt + minRenderInterval)) {
    renderFrame();
    lastRenderAt = now;
  }
}

bool Runtime::pollInputSources() {
  bool queuedInput = false;
  for (size_t i = 0; i < inputSourceCount; ++i) {
    if (inputSources[i] == nullptr) {
      continue;
    }

    for (size_t eventIndex = 0; eventIndex < maxInputEventsPerSourcePerPump;
         ++eventIndex) {
      Optional<InputEvent> event = inputSources[i]->pollEvent();
      if (!event.has_value()) {
        break;
      }

      if (!postInputEvent(event.value())) {
        return queuedInput;
      }

      queuedInput = true;
    }
  }

  return queuedInput;
}

bool Runtime::dispatchEvents() {
  bool changed = false;

  while (true) {
    Optional<InputEvent> event = eventQueue.pop();
    if (!event.has_value()) {
      break;
    }

    updatePointerPresentation(event.value());
    changed = screenManager.handleInput(event.value()) || changed;
  }

  return changed;
}

bool Runtime::advanceUiState(const Time::Duration &now) {
  bool changed = screenManager.tick(now);
  if (footer != nullptr && footer->tick(now)) {
    while (true) {
      Optional<Rect> footerInvalidation = footer->takeInvalidation();
      if (!footerInvalidation.has_value()) {
        break;
      }

      invalidate(footerInvalidation.value());
    }
  }
  return changed;
}

bool Runtime::drainViewInvalidations() {
  bool invalidated = false;

  View *screen = screenManager.activeScreen();
  if (screen != nullptr) {
    Optional<Rect> region = screen->takeInvalidation();
    if (region.has_value()) {
      invalidate(region.value());
      invalidated = true;
    }
  }

  Overlay *overlay = screenManager.activeOverlay();
  if (overlay != nullptr) {
    Optional<Rect> region = overlay->takeInvalidation();
    while (region.has_value()) {
      invalidate(region.value());
      invalidated = true;
      region = overlay->takeInvalidation();
    }
  }

  return invalidated;
}

void Runtime::updatePointerPresentation(const InputEvent &event) {
  if (event.type != InputEventType::PointerDown &&
      event.type != InputEventType::PointerMove &&
      event.type != InputEventType::PointerUp) {
    return;
  }

  const PointerPresentation previousPointer = pointerPresentation;
  Overlay *overlay = touchIndicatorOverlay;
  TouchIndicatorOverlay *touchOverlay =
      static_cast<TouchIndicatorOverlay *>(overlay);

  switch (event.type) {
  case InputEventType::PointerDown:
  case InputEventType::PointerMove:
    pointerPresentation.position = event.position;
    pointerPresentation.visible = true;
    break;
  case InputEventType::PointerUp:
    pointerPresentation.position = event.position;
    pointerPresentation.visible = false;
    break;
  default:
    break;
  }

  if (touchOverlay != nullptr) {
    touchOverlay->syncTransition(previousPointer, pointerPresentation,
                                 uiSettings.showTouchIndicator);
  }
}

void Runtime::renderFrame() {
  if (!ready || surface == nullptr) {
    return;
  }

  if (fullRedrawPending) {
    renderer.render(*surface, screenManager, footer);
  } else {
    for (size_t i = 0; i < dirtyRectCount; ++i) {
      renderDirtyRegion(*surface, screenManager, footer, dirtyRects[i]);
    }
  }

  fullRedrawPending = false;
  dirtyRectCount = 0;
}

} // namespace UI
