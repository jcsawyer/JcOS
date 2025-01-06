#pragma once

#include "_arch/cpu.hpp"
#include "print.hpp"

static bool already_panicking = false;

[[noreturn]] void panic() {
  if (already_panicking) {
    warn("\n\tPANIC in PANIC\n");
  }

  already_panicking = true;

  panic("KERNEL PANIC\n\n");

  CPU::waitForever();
}
