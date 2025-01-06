#include "memory.h"

extern "C" void *memset(void *dest, int value, size_t count) {
  unsigned char *ptr = static_cast<unsigned char *>(dest);
  while (count--) {
    *ptr++ = static_cast<unsigned char>(value);
  }
  return dest;
}
