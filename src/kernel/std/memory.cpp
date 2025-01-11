#include <memory.h>

extern "C" void *memset(void *dest, const int value, size_t count) {
  auto ptr = static_cast<unsigned char *>(dest);
  while (count--) {
    *ptr++ = static_cast<unsigned char>(value);
  }
  return dest;
}
