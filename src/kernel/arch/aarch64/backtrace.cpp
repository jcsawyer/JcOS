#include "backtrace.hpp"

#include <bsp/raspberrypi/memory.hpp>

namespace Backtrace::Arch {
namespace {

struct StackFrameRecord {
  const StackFrameRecord *previousRecord;
  uintptr_t link;
};

struct WalkState {
  const StackFrameRecord *current;
  bool abortAfterCurrent;
};

bool isValidFrameRecord(const StackFrameRecord *record) {
  uintptr_t address = reinterpret_cast<uintptr_t>(record);
  if ((address & 0xFUL) != 0) {
    return false;
  }

  return Memory::isValidCurrentStackAddress(address) &&
         Memory::isValidCurrentStackAddress(address + sizeof(StackFrameRecord) -
                                            1);
}

bool advance(WalkState &state, BacktraceItem &item) {
  if (state.current == nullptr) {
    return false;
  }

  if (state.abortAfterCurrent) {
    state.current = nullptr;
    return false;
  }

  const StackFrameRecord *previous = state.current->previousRecord;
  if (previous == nullptr) {
    return false;
  }

  if (!isValidFrameRecord(previous)) {
    item = {ItemType::InvalidFramePointer,
            reinterpret_cast<uintptr_t>(previous)};
    state.abortAfterCurrent = true;
    return true;
  }

  uintptr_t link = state.current->link;
  if (!Memory::isValidCodeAddress(link)) {
    item = {ItemType::InvalidLink, link};
  } else {
    if (link >= 4) {
      link -= 4;
    }
    item = {ItemType::Link, link};
  }

  state.current = previous;
  return true;
}

} // namespace

void walk(WalkCallback callback, void *context) {
  uintptr_t fp = 0;
  asm volatile("mov %0, x29" : "=r"(fp));
  const StackFrameRecord *current =
      reinterpret_cast<const StackFrameRecord *>(fp);
  if (!isValidFrameRecord(current)) {
    return;
  }

  WalkState state{current, false};
  BacktraceItem item{};
  size_t frameIndex = 0;

  while (advance(state, item)) {
    if (!callback(item, frameIndex++, context)) {
      return;
    }
  }
}

} // namespace Backtrace::Arch
