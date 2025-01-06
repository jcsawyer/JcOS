#include "print.hpp"

void info(const char *format, Args... args) {
  Console::Console *console = Console::Console::GetInstance();
  // Time::TimeManager *timeManager = Time::TimeManager::GetInstance();

  // Time::Duration uptime = timeManager->uptime();
  printyf("hello %s", args...);

  // console->print("[ %03d.%06d ] ", uptime.as_secs(), uptime.subsec_micros());
  // console->print(format, args...);
  // console->print("\n");
}

void warn(const char *format, Args... args) {
  Console::Console *console = Console::Console::GetInstance();
  // Time::TimeManager *timeManager = Time::TimeManager::GetInstance();

  // Time::Duration uptime = timeManager->uptime();

  // console->print("[W%03d.%06d ] ", uptime.as_secs(), uptime.subsec_micros());
  console->print(format, args...);
  console->print("\n");
}

void panic(const char *format, Args... args) {
  Console::Console *console = Console::Console::GetInstance();
  // Time::TimeManager *timeManager = Time::TimeManager::GetInstance();

  // Time::Duration uptime = timeManager->uptime();

  // console->print("[P%03d.%06d ] ", uptime.as_secs(), uptime.subsec_micros());
  console->print(format, args...);
  console->print("\n");
}
