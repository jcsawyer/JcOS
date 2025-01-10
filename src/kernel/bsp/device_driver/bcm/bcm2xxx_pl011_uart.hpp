#pragma once

#include "../../../console/console.hpp"
#include "../../../driver/driver.hpp"
#include <stdint.h>

namespace Driver::BSP::BCM {
class UART : public Driver::DeviceDriver {
public:
  UART(uint32_t mmio_start_addr) : registerBlock(mmio_start_addr) {};
  const char *compatible() override { return "BCM PL011 UART"; }
  void init() override;
  void putc(const char c);
  char getc();
  void flush();
  class UartConsole : public Console::Console {
  public:
    UartConsole(UART *uart) : uart(uart) {};
    void flush();
    void clearRx();
    void print(const char *s, ...);
    void printChar(char c);
    void printLine(const char *s, ...);
    char readChar();

  private:
    UART *uart;
  };

private:
  class RegisterBlock {
  public:
    volatile uint32_t *DR;
    volatile uint32_t *FR;
    volatile uint32_t *IBRD;
    volatile uint32_t *FBRD;
    volatile uint32_t *LCRH;
    volatile uint32_t *CR;
    volatile uint32_t *IMSC;
    volatile uint32_t *ICR;
    volatile uint32_t *MBOX_READ;
    volatile uint32_t *MBOX_STATUS;
    volatile uint32_t *MBOX_WRITE;

    RegisterBlock(unsigned int mmio_start_addr) {
      DR = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x00);
      FR = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x18);
      IBRD = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x24);
      FBRD = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x28);
      LCRH = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x2C);
      CR = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x30);
      IMSC = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x38);
      ICR = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x44);
      MBOX_READ = reinterpret_cast<volatile uint32_t *>(0xB880 + 0x00);
      MBOX_STATUS = reinterpret_cast<volatile uint32_t *>(0xB880 + 0x18);
      MBOX_WRITE = reinterpret_cast<volatile uint32_t *>(0xB880 + 0x20);
    }
  };
  RegisterBlock registerBlock;
};
} // namespace Driver::BSP::BCM
