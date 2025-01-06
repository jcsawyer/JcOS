#pragma once
#include "stddef.h"

extern "C" void *memset(void *dest, int value, size_t count);

// Define `operator delete` for your freestanding environment.
inline void operator delete(void *ptr, size_t) noexcept {
  // No-op or custom cleanup logic for your environment.
}

inline void operator delete(void *ptr) noexcept {
  // No-op or custom cleanup logic for your environment.
}
