#pragma once
#include <stdio/printf.h>

const size_t KB = 1000;
const size_t MB = 1000 * 1000;
const size_t GB = 1000 * 1000 * 1000;

const size_t KiB = 1024;
const size_t MiB = 1024 * 1024;
const size_t GiB = 1024 * 1024 * 1024;

inline const size_t div_ceil(size_t value, size_t divisor) {
  return (value + divisor - 1) / divisor;
}

inline const size_t div_floor(size_t a, size_t b) {
  return (a > 0 && b < 0)   ? ((a - 1) / b) - 1
         : (a < 0 && b > 0) ? (a + 1) / b - 1
                            : a / b;
}

inline const char *size_human_readable_ceil(size_t s) {
  size_t size = s;
  char *unit = "\0";

  if ((s / GiB) > 0) {
    size = div_ceil(s, GiB);
    unit = "GiB";
  } else if ((s / MiB) > 0) {
    size = div_ceil(s, MiB);
    unit = "MiB";
  } else if ((s / KiB) > 0) {
    size = div_ceil(s, KiB);
    unit = "KiB";
  } else {
    unit = "Byte";
  }

  char *output;
  snprintf_(output, 10, "%d %s", size, unit);
  return output;
}

inline const char *size_human_readable_ceil_bytes(size_t s) {
  size_t size = s;
  char *unit = "\0";

  if ((s / GB) > 0) {
    size = div_floor(s, GB);
    unit = "GB";
  } else if ((s / MB) > 0) {
    size = div_floor(s, MB);
    unit = "MB";
  } else if ((s / KB) > 0) {
    size = div_floor(s, KB);
    unit = "KB";
  } else {
    unit = "Byte";
  }

  char *output;
  snprintf_(output, 10, "%d %s", size, unit);
  return output;
}