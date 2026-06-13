#pragma once

#include "lcd.hpp"

namespace Chainloader {

class BootDisplay {
public:
  explicit BootDisplay(LCD &lcd);

  void showStage(const char *line1, const char *line2 = nullptr);
  void showError(const char *code, const char *detail = nullptr);

private:
  LCD &lcd_;

  void writeLine(unsigned char row, const char *text);
};

} // namespace Chainloader
