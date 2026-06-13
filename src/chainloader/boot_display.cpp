#include "boot_display.hpp"

#include "board_config.hpp"

#include <stdarg.h>
#include <stdio/printf.h>

namespace Chainloader {

namespace {
void formatLine(char *buffer, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vsnprintf_(buffer, Board::LCD_COLUMNS + 1, format, args);
  va_end(args);
}
} // namespace

BootDisplay::BootDisplay(LCD &lcd) : lcd_(lcd) {}

void BootDisplay::writeLine(unsigned char row, const char *text) {
  char line[Board::LCD_COLUMNS + 1];
  size_t i = 0;

  for (; i < Board::LCD_COLUMNS && text != nullptr && text[i] != '\0'; ++i) {
    line[i] = text[i];
  }

  for (; i < Board::LCD_COLUMNS; ++i) {
    line[i] = ' ';
  }
  line[Board::LCD_COLUMNS] = '\0';

  lcd_.setCursor(row, 0);
  lcd_.writeString(line);
}

void BootDisplay::showStage(const char *line1, const char *line2) {
  writeLine(0, line1 == nullptr ? "" : line1);
  writeLine(1, line2 == nullptr ? "" : line2);
}

void BootDisplay::showError(const char *code, const char *detail) {
  char secondLine[Board::LCD_COLUMNS + 1];
  if (detail == nullptr || detail[0] == '\0') {
    formatLine(secondLine, "ERR/%s", code);
  } else {
    formatLine(secondLine, "%s/%s", code, detail);
  }

  showStage("ERR", secondLine);
}

} // namespace Chainloader
