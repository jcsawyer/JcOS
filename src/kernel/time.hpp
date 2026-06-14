#pragma once

#include <stdint.h>
#include <time/duration.hpp>

namespace Time {

class TimeManager {
public:
  using TimeoutCallback = void (*)();

  static TimeManager *GetInstance();

  void earlyInit();

  Time::Duration resolution() const;

  Time::Duration uptime() const;

  void spinFor(const Time::Duration &duration);

  void setTimeoutOnce(const Time::Duration &delay, TimeoutCallback callback);
  void setTimeoutPeriodic(const Time::Duration &delay,
                          TimeoutCallback callback);
  bool handleTimeoutIrq();

private:
  struct Timeout {
    Time::Duration dueTime;
    bool periodic;
    Time::Duration period;
    TimeoutCallback callback;

    Timeout()
        : dueTime(Time::Duration::zero()), periodic(false),
          period(Time::Duration::zero()), callback(nullptr) {}

    Timeout(const Time::Duration &dueTime, bool periodic,
            const Time::Duration &period, TimeoutCallback callback)
        : dueTime(dueTime), periodic(periodic), period(period),
          callback(callback) {}

    void refresh() {
      if (periodic) {
        dueTime += period;
      }
    }
  };

  TimeManager();

  struct Impl;
  Impl *impl;
  static TimeManager *instance;

  void setTimeout(const Timeout &timeout);
};

void registerTimerDriver();

} // namespace Time
