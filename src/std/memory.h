#pragma once
#include "cstddef.h"

// Define `operator delete` for your freestanding environment.
void operator delete(void *ptr, size_t) noexcept {
  // No-op or custom cleanup logic for your environment.
}

void operator delete(void *ptr) noexcept {
  // No-op or custom cleanup logic for your environment.
}
