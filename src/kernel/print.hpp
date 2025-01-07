#pragma once
#include "console/console.hpp"
#include "time.hpp"
#include <duration.hpp>

template <class... Args> void info(const char *format, Args... args) {
  Console::Console *console = Console::Console::GetInstance();
  Time::TimeManager *timeManager = Time::TimeManager::GetInstance();

  Time::Duration uptime = timeManager->uptime();

  console->print("[ %03d.%06d ] ", uptime.as_secs(), uptime.subsec_micros());
  console->print(format, args...);
  console->print("\n");
}

template <class... Args> void warn(const char *format, Args... args) {
  Console::Console *console = Console::Console::GetInstance();
  Time::TimeManager *timeManager = Time::TimeManager::GetInstance();

  Time::Duration uptime = timeManager->uptime();

  console->print("[W%03d.%06d ] ", uptime.as_secs(), uptime.subsec_micros());
  console->print(format, args...);
  console->print("\n");
}

template <class... Args> void panic(const char *format, Args... args) {
  Console::Console *console = Console::Console::GetInstance();
  Time::TimeManager *timeManager = Time::TimeManager::GetInstance();

  Time::Duration uptime = timeManager->uptime();

  console->print("[P%03d.%06d ] ", uptime.as_secs(), uptime.subsec_micros());
  console->print(format, args...);
  console->print("\n");
}
