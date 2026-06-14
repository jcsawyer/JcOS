#pragma once
#include <stddef.h>

extern "C" void *memset(void *dest, int value, size_t count);

namespace std {
enum class align_val_t : size_t {};
}

void *operator new(size_t size);
void *operator new[](size_t size);
void *operator new(size_t size, std::align_val_t alignment);
void *operator new[](size_t size, std::align_val_t alignment);

void operator delete(void *ptr) noexcept;
void operator delete[](void *ptr) noexcept;
void operator delete(void *ptr, size_t size) noexcept;
void operator delete[](void *ptr, size_t size) noexcept;
void operator delete(void *ptr, std::align_val_t alignment) noexcept;
void operator delete[](void *ptr, std::align_val_t alignment) noexcept;
void operator delete(void *ptr, size_t size,
                     std::align_val_t alignment) noexcept;
void operator delete[](void *ptr, size_t size,
                       std::align_val_t alignment) noexcept;

inline void *operator new(size_t, void *ptr) noexcept { return ptr; }
inline void operator delete(void *, void *) noexcept {} // no-op
