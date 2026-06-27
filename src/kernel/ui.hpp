#pragma once

#include <display/display.hpp>
#include <bsp/device_driver/lcd/touch_controller.hpp>
#include <optional.hpp>
#include <stddef.h>
#include <stdint.h>
#include <time/duration.hpp>

namespace UI {

struct Point {
  int x = 0;
  int y = 0;
};

struct Size {
  unsigned int width = 0;
  unsigned int height = 0;
};

struct Rect {
  int x = 0;
  int y = 0;
  unsigned int width = 0;
  unsigned int height = 0;

  int right() const { return x + static_cast<int>(width); }
  int bottom() const { return y + static_cast<int>(height); }
  bool empty() const { return width == 0 || height == 0; }
};

enum class InputEventType {
  NavigateUp,
  NavigateDown,
  NavigateLeft,
  NavigateRight,
  Select,
  Back,
  PointerDown,
  PointerMove,
  PointerUp,
  Tick,
};

struct InputEvent {
  InputEventType type = InputEventType::Tick;
  Point position{};
  Time::Duration timestamp = Time::Duration::zero();
};

class InputSource {
public:
  virtual ~InputSource() = default;
  virtual Optional<InputEvent> pollEvent() = 0;
};

class TouchInputSource : public InputSource {
public:
  TouchInputSource(Driver::BSP::Touch::TouchPanel *touchPanel,
                   Driver::Display::Display *display);

  Optional<InputEvent> pollEvent() override;

private:
  Driver::BSP::Touch::TouchPanel *touchPanel;
  Driver::Display::Display *display;
  bool pointerActive = false;
  bool tapEligible = false;
  bool selectPending = false;
  Point pressPoint{};
  Point lastPoint{};
  Point releasePoint{};
  Time::Duration pressStartedAt = Time::Duration::zero();
  Time::Duration lastTouchSampleAt = Time::Duration::zero();
  Time::Duration lastTouchPollAt = Time::Duration::zero();

  Point mapToDisplay(const Driver::BSP::Touch::Sample &sample) const;
  static bool movedFarEnough(const Point &lhs, const Point &rhs,
                             unsigned int threshold);
};

class Surface {
public:
  explicit Surface(Driver::Display::Display *display);

  Size size() const;
  Rect bounds() const;

  void setClipRect(const Rect &rect);
  void resetClipRect();

  void clear(uint16_t color);
  void fillRect(const Rect &rect, uint16_t color);
  void drawMonoGlyph(const Point &origin, char value, uint16_t foreground,
                     uint16_t background, unsigned int scale = 1);
  void drawString(const Point &origin, const char *text, uint16_t foreground,
                  uint16_t background, unsigned int scale = 1);

private:
  Driver::Display::Display *display;
  Rect clipRect;

  Rect clipped(const Rect &rect) const;
};

class View {
public:
  virtual ~View() = default;
  virtual void layout(const Rect &bounds) = 0;
  virtual void render(Surface &surface) = 0;
  virtual bool handleInput(const InputEvent &event) {
    (void)event;
    return false;
  }
  virtual bool tick(const Time::Duration &now) {
    (void)now;
    return false;
  }
  virtual Optional<Rect> takeInvalidation() { return Optional<Rect>(); }
};

class Overlay : public View {};

class StatusFooter;

class ScreenManager {
public:
  void setRootScreen(View *view);
  bool pushScreen(View *view);
  void popScreen();
  bool pushOverlay(Overlay *overlay);
  void popOverlay();

  View *activeScreen() const;
  Overlay *activeOverlay() const;

  void layout(const Rect &screenBounds, const Rect &overlayBounds);
  bool handleInput(const InputEvent &event);
  bool tick(const Time::Duration &now);

private:
  static constexpr size_t maxScreens = 4;
  static constexpr size_t maxOverlays = 4;

  View *screens[maxScreens] = {};
  size_t screenCount = 0;
  Overlay *overlays[maxOverlays] = {};
  size_t overlayCount = 0;
};

class Renderer {
public:
  void render(Surface &surface, ScreenManager &screenManager,
              StatusFooter *footer);
};

class Runtime {
public:
  static Runtime *GetInstance();

  void init(Driver::Display::Display *display);
  void start();

  bool isReady() const { return ready; }
  void invalidate(const Rect &rect);
  void invalidateAll();
  bool postInputEvent(const InputEvent &event);
  bool registerInputSource(InputSource *inputSource);

private:
  static constexpr size_t maxDirtyRects = 8;
  static constexpr size_t maxInputEvents = 16;
  static constexpr size_t maxInputSources = 4;

  struct EventQueue {
    InputEvent events[maxInputEvents] = {};
    size_t readIndex = 0;
    size_t writeIndex = 0;
    size_t count = 0;

    bool push(const InputEvent &event);
    Optional<InputEvent> pop();
  };

  Runtime() = default;

  static void taskEntry();

  void pump();
  bool pollInputSources();
  bool dispatchEvents();
  bool advanceUiState(const Time::Duration &now);
  bool drainViewInvalidations();
  void renderFrame();

  Driver::Display::Display *display = nullptr;
  Surface *surface = nullptr;
  View *homeScreen = nullptr;
  StatusFooter *footer = nullptr;
  ScreenManager screenManager;
  Renderer renderer;
  EventQueue eventQueue;
  Rect dirtyRects[maxDirtyRects] = {};
  size_t dirtyRectCount = 0;
  InputSource *inputSources[maxInputSources] = {};
  size_t inputSourceCount = 0;
  Time::Duration lastTickAt = Time::Duration::zero();
  Time::Duration lastRenderAt = Time::Duration::zero();
  bool ready = false;
  bool started = false;
  bool fullRedrawPending = false;
};

} // namespace UI
