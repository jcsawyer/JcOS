#pragma once
#include <stdint.h>

namespace Memory {
template <typename TRegister> class InMemoryRegister {
public:
  static constexpr uint64_t get() { return TRegister::get(); }

  static constexpr void set(uint64_t value) { TRegister::write(value); }
};
} // namespace Memory