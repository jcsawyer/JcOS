#pragma once

#include <arch/aarch64/exception/asynchronous.hpp>
#include <state.hpp>

namespace Syncrhonization {

template <typename T> class IRQSafeNullLock {
public:
  constexpr IRQSafeNullLock(T data) : data_(data) {}

  template <typename Func> auto lock(Func f) const {
    return exec_with_irq_masked([&]() -> decltype(auto) { return f(data_); });
  }

private:
  mutable T data_;
};

// Lock for init-phase data: writable only during single-core boot
template <typename T> class InitStateLock {
public:
  constexpr InitStateLock(T data) : data_(data) {}

  template <typename Func> auto write(Func f) const {
    if (!state_manager().is_init()) {
      panic("InitStateLock::write called after init");
    }

    if (!Exception::Asynchronous::isLocalIrqMasked()) {
      panic("InitStateLock::write called with IRQs unmasked");
    }

    return f(data_);
  }

  template <typename Func> auto read(Func f) const { return f(data_); }

private:
  mutable T data_;
};

} // namespace Syncrhonization
