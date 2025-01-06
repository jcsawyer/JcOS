#pragma once
#include "console/console.hpp"
#include "std/duration.hpp"
#include "stdarg.h"
#include "time.hpp"
void printyc(char c) {
  Console::Console *console = Console::Console::GetInstance();
  console->printChar(c);
}

void printy(const char *s) {
  while (*s != '\0') {
    printyc(*s);
    s++;
  }
}

int printyf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const char *text;
  char c;
  while (*fmt) {
    if ((c = *fmt++) != '%') {
      printyc(c);
      continue;
    }
    switch ((c = *fmt++)) {
    case 's':
      printy(va_arg(ap, const char *));
      break;
    default:
      printyc(c);
    }
  }
  va_end(ap);
  return 0;
}
