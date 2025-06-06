#pragma once
#include <console/console.hpp>
#include <time.hpp>
#include <time/duration.hpp>

template <class... Args> void info(const char *format, Args... args) {
  Console::Console *console = Console::Console::GetInstance();
  Time::TimeManager *timeManager = Time::TimeManager::GetInstance();

  Time::Duration uptime = timeManager->uptime();

  console->print("[ %03d.%06d ] ", uptime.as_secs(), uptime.subsec_micros());
  console->print(format, args...);
  console->print("\n");
}

template <class... Args> void debug(const char *format, Args... args) {
  Console::Console *console = Console::Console::GetInstance();
  // if (!console->isDebugMode) {
  // return;
  // }
  Time::TimeManager *timeManager = Time::TimeManager::GetInstance();

  Time::Duration uptime = timeManager->uptime();

  console->print("[D%03d.%06d ] ", uptime.as_secs(), uptime.subsec_micros());
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
