#pragma once

#include "arch/cpu.hpp"
#include "backtrace.hpp"
#include "console/console.hpp"
#include <print.hpp>

static bool already_panicking = false;

template <class... Args>
inline void panicPrint(const char *format, const char *message, Args... args) {
  Console::Console *console = Console::Console::GetInstance();

  console->print("\n");
  console->print("[PANIC] ");
  console->print(message, args...);

  console->print(format);
}

template <class... Args>
[[noreturn]] inline constexpr void panic(const char *message, Args... args) {
  if (already_panicking) {
    panicPrint("\n\t\t!!PANIC in PANIC!!\n", message, args...);
    CPU::waitForever();
  }

  already_panicking = true;

  panicPrint("\n\t!KERNEL PANIC!\n", message, args...);
  Backtrace::print();

  CPU::waitForever();
}
