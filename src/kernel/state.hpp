#pragma once

#include <panic.hpp>
#include <stdint.h>

namespace State {

enum class KernelState : char {
  Init = 0,
  SingleCoreMain = 1,
  MultiCoreMain = 2,
};

class VolatileU8 {
public:
  constexpr VolatileU8(uint32_t value = 0) : value_(value) {}

  uint32_t load() const { return __atomic_load_n(&value_, __ATOMIC_ACQUIRE); }

  void store(uint32_t value) {
    __atomic_store_n(&value_, value, __ATOMIC_RELEASE);
  }

  bool compare_exchange(uint32_t expected, uint32_t desired) {
    return __atomic_compare_exchange_n(&value_, &expected, desired, false,
                                       __ATOMIC_ACQUIRE, __ATOMIC_RELAXED);
  }

private:
  volatile uint32_t value_;
};
class StateManager {
public:
  constexpr StateManager() : state_(static_cast<char>(KernelState::Init)) {}

  bool is_init() const { return current_state() == KernelState::Init; }

  void transition_to_single_core_main() {
    if (!state_.compare_exchange(
            static_cast<char>(KernelState::Init),
            static_cast<char>(KernelState::SingleCoreMain))) {
      panic("Invalid transition: not in Init state");
    }
  }

  void transition_to_multi_core_main() {
    if (!state_.compare_exchange(
            static_cast<char>(KernelState::SingleCoreMain),
            static_cast<char>(KernelState::MultiCoreMain))) {
      panic("Invalid transition: not in SingleCoreMain state");
    }
  }

private:
  VolatileU8 state_;

  KernelState current_state() const {
    char raw = state_.load();
    switch (raw) {
    case 0:
      return KernelState::Init;
    case 1:
      return KernelState::SingleCoreMain;
    case 2:
      return KernelState::MultiCoreMain;
    default:
      panic("Invalid kernel state");
    }
  }
};

inline StateManager &state_manager() {
  static StateManager instance;
  return instance;
}

} // namespace State