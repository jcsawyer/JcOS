#pragma once

#include "arch/time.hpp"
#include <stdint.h>
#include <time/duration.hpp>

namespace Time {
class TimeManager {
public:
  static TimeManager *GetInstance();
  void init() { Time::Arch::init(); }

  Time::Duration resolution() const { return Time::Arch::resolution(); }

  Time::Duration uptime() const { return Time::Arch::uptime(); }

  void spinFor(const Time::Duration &duration) {
    Time::Arch::spinFor(duration);
  }

private:
  static TimeManager *instance;
};
} // namespace Time
