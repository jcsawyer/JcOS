#pragma once

#include "_arch/cpu.hpp"

static bool already_panicking = false;

template <class... Args> inline void panicPrint(const char *format, const char* message, Args... args) {
  Console::Console *console = Console::Console::GetInstance();
  Time::TimeManager *timeManager = Time::TimeManager::GetInstance();

  Time::Duration uptime = timeManager->uptime();

  console->print("[P%03d.%06d ] ", uptime.as_secs(), uptime.subsec_micros());
  console->print(message, args...);
  console->print("\n");
}

template <class... Args> [[noreturn]] inline void panic(const char* message, Args... args) {
  if (already_panicking) {
    warn("\n\tPANIC in PANIC\n");
    CPU::waitForever();
  }

  already_panicking = true;

  panicPrint("KERNEL PANIC\n\n", message, args...);

  CPU::waitForever();
}