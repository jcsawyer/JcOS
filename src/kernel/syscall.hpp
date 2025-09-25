#pragma once
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio/printf.h>

namespace Syscall {

const uint64_t WRITE = 0;
const uint64_t EXIT = 1;

extern "C" uint64_t syscall1(uint64_t nr, uint64_t a);
extern "C" uint64_t syscall3(uint64_t nr, uint64_t a, uint64_t b, uint64_t c);

static inline void write(const char *format, ...) {
  char buf[512];
  va_list args;
  va_start(args, format);
  int len = vsnprintf_(buf, sizeof(buf), format, args);
  if (len < 0)
    len = 0;
  if ((size_t)len > sizeof(buf))
    len = sizeof(buf);
  va_end(args);
  syscall3(WRITE, (uint64_t)&buf, (uint64_t)len, 0);
}

static inline void exit(int code) { syscall1(EXIT, code); }

} // namespace Syscall
