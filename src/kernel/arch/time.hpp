#pragma once
#include <stdint.h>
#include <time/duration.hpp>

namespace Time {
namespace Arch {
const uint64_t NANOSEC_PER_SEC = 1000000000;

enum class TimeError { None, DurationTooBig };

class GenericTimerCounterValue {
public:
  uint64_t value;

  static const GenericTimerCounterValue MAX;

  GenericTimerCounterValue(uint64_t value) : value(value) {}

  GenericTimerCounterValue add(const GenericTimerCounterValue &other) const {
    return GenericTimerCounterValue(value + other.value);
  }

  static GenericTimerCounterValue fromDuration(const Duration &duration,
                                               TimeError &error);
  Duration toDuration() const;

private:
  static uint32_t archTimerCounterFrequency();
};

void init();
Time::Duration resolution();
Time::Duration uptime();
void spinFor(const Time::Duration &duration);
} // namespace Arch
} // namespace Time
