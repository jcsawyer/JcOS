#pragma once

#include <stddef.h>
#include <stdint.h>

namespace Backtrace {

enum class ItemType { InvalidFramePointer, InvalidLink, Link };

struct BacktraceItem {
  ItemType type;
  uintptr_t address;
};

void print();

} // namespace Backtrace
