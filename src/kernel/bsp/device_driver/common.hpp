#pragma once

#include <memory.h>
#include <optional.hpp>
#include <panic.hpp>
#include <stdint.h>

namespace BSP::Common {

template <typename T> class MMIODerefWrapper {
public:
  constexpr explicit MMIODerefWrapper(uintptr_t start_addr)
      : _start_addr(start_addr) {}

  T &operator*() const { return *reinterpret_cast<T *>(_start_addr); }

  T *operator->() const { return reinterpret_cast<T *>(_start_addr); }

private:
  uintptr_t _start_addr;
};

template <unsigned long MaxInclusive> class BoundedUsize {
public:
  static constexpr unsigned long MAX_INCLUSIVE = MaxInclusive;

  static constexpr auto newBounded(unsigned long value)
      -> Optional<BoundedUsize> {
    if (value > MaxInclusive) {
      return {};
    }
    return BoundedUsize(value);
  }

  constexpr unsigned long get() const { return _value; }

  constexpr bool operator==(const BoundedUsize &other) const {
    return _value == other._value;
  }

private:
  unsigned long _value;

  constexpr explicit BoundedUsize(unsigned long value) : _value(value) {}

public:
  constexpr BoundedUsize() = delete;
};

} // namespace BSP::Common