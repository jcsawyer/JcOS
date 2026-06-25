#pragma once
#include <console/console.hpp>
#include <exceptions/asynchronous.hpp>
#include <time.hpp>
#include <time/duration.hpp>

template <class... Args> void info(const char *format, Args... args) {
  Exceptions::Asynchronous::execWithIrqMasked([&]() {
    Console::Console *console = Console::Console::GetInstance();
    Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
    Time::Duration uptime = timeManager->uptime();

    console->print("[ %03lu.%06lu ] ",
                   static_cast<unsigned long>(uptime.as_secs()),
                   static_cast<unsigned long>(uptime.subsec_micros()));
    console->print(format, args...);
    console->print("\n");
  });
}

template <class... Args> void debug(const char *format, Args... args) {
#ifdef DEBUG_PRINTS
  Exceptions::Asynchronous::execWithIrqMasked([&]() {
    Console::Console *console = Console::Console::GetInstance();
    Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
    Time::Duration uptime = timeManager->uptime();

    console->print("[D%03lu.%06lu ] ",
                   static_cast<unsigned long>(uptime.as_secs()),
                   static_cast<unsigned long>(uptime.subsec_micros()));
    console->print(format, args...);
    console->print("\n");
  });
#else
  (void)format;
  (void)sizeof...(args);
#endif
}

template <class... Args> void warn(const char *format, Args... args) {
  Exceptions::Asynchronous::execWithIrqMasked([&]() {
    Console::Console *console = Console::Console::GetInstance();
    Time::TimeManager *timeManager = Time::TimeManager::GetInstance();
    Time::Duration uptime = timeManager->uptime();

    console->print("[W%03lu.%06lu ] ",
                   static_cast<unsigned long>(uptime.as_secs()),
                   static_cast<unsigned long>(uptime.subsec_micros()));
    console->print(format, args...);
    console->print("\n");
  });
}
