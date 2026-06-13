#pragma once

#include <stdint.h>

namespace Chainloader {

class Timer {
public:
  static uint32_t frequency();
  static uint64_t ticks();
  static uint64_t microsToTicks(uint64_t micros);
  static bool expired(uint64_t startTicks, uint64_t timeoutMicros);
  static void delayMicros(uint64_t micros);
  static void delayMillis(uint64_t millis);
};

} // namespace Chainloader
