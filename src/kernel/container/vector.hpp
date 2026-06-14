#pragma once

#include <memory.h>
#include <panic.hpp>
#include <stddef.h>

namespace Container {
namespace Detail {
template <typename T> constexpr T &&move(T &value) {
  return static_cast<T &&>(value);
}
} // namespace Detail

template <typename T> class Vector {
public:
  constexpr Vector() = default;

  Vector(const Vector &) = delete;
  Vector &operator=(const Vector &) = delete;

  Vector(Vector &&other) noexcept
      : elements(other.elements), count(other.count),
        capacity_(other.capacity_) {
    other.elements = nullptr;
    other.count = 0;
    other.capacity_ = 0;
  }

  Vector &operator=(Vector &&other) noexcept {
    if (this == &other) {
      return *this;
    }

    clear();
    ::operator delete(elements);

    elements = other.elements;
    count = other.count;
    capacity_ = other.capacity_;

    other.elements = nullptr;
    other.count = 0;
    other.capacity_ = 0;

    return *this;
  }

  ~Vector() {
    clear();
    ::operator delete(elements);
  }

  void pushBack(const T &value) {
    ensureCapacity(count + 1);
    new (&elements[count]) T(value);
    ++count;
  }

  void pushBack(T &&value) {
    ensureCapacity(count + 1);
    new (&elements[count]) T(Detail::move(value));
    ++count;
  }

  void resize(size_t newSize, const T &value) {
    if (newSize < count) {
      while (count > newSize) {
        --count;
        elements[count].~T();
      }
      return;
    }

    ensureCapacity(newSize);
    while (count < newSize) {
      new (&elements[count]) T(value);
      ++count;
    }
  }

  size_t size() const { return count; }

  bool empty() const { return count == 0; }

  T &operator[](size_t index) { return elements[index]; }

  const T &operator[](size_t index) const { return elements[index]; }

  T *begin() { return elements; }
  T *end() { return elements + count; }
  const T *begin() const { return elements; }
  const T *end() const { return elements + count; }

private:
  void clear() {
    while (count > 0) {
      --count;
      elements[count].~T();
    }
  }

  void ensureCapacity(size_t minCapacity) {
    if (minCapacity <= capacity_) {
      return;
    }

    size_t newCapacity = capacity_ == 0 ? 4 : capacity_ * 2;
    while (newCapacity < minCapacity) {
      newCapacity *= 2;
    }

    T *newElements = static_cast<T *>(::operator new(sizeof(T) * newCapacity));
    if (newElements == nullptr) {
      panic("Vector allocation failed");
    }

    for (size_t i = 0; i < count; ++i) {
      new (&newElements[i]) T(Detail::move(elements[i]));
      elements[i].~T();
    }

    ::operator delete(elements);
    elements = newElements;
    capacity_ = newCapacity;
  }

  T *elements = nullptr;
  size_t count = 0;
  size_t capacity_ = 0;
};

} // namespace Container
