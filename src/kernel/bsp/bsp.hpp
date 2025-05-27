#pragma once

#include <stddef.h>

namespace BSP {
class Board {
public:
  static void init();
  static void PrintInfo();
  static size_t getDefaultLoadAddr();
};
} // namespace BSP