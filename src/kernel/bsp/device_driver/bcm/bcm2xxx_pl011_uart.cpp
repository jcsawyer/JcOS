#include "bcm2xxx_pl011_uart.hpp"
#include "../../../_arch/cpu.hpp"
#include <printf.h>

namespace Driver::BSP::BCM {
volatile uint32_t *MBOX_STATUS =
    reinterpret_cast<volatile uint32_t *>(0x3F00B898);
volatile uint32_t *MBOX_WRITE =
    reinterpret_cast<volatile uint32_t *>(0x3F00B8A0);
volatile uint32_t *MBOX_READ =
    reinterpret_cast<volatile uint32_t *>(0x3F00B880);

const uint32_t MBOX_FULL = 0x80000000;
const uint32_t MBOX_EMPTY = 0x40000000;
const uint32_t MBOX_RESPONSE = 0x80000000;

alignas(16) uint32_t mbox[36];

int call(uint8_t ch) {
  const uint32_t mask = 0x0F;
  uintptr_t ptr = reinterpret_cast<uint64_t>(&mbox);
  uintptr_t r = (ptr & ~mask) | (ch & 0xF);

  // Wait until we can write to the mailbox
  while ((*MBOX_STATUS & MBOX_FULL) != 0) {
    asm volatile("nop");
  }

  // Write the address of our message to the mailbox with channel identifier
  *MBOX_WRITE = static_cast<uint32_t>(r);

  // Wait for the response
  while (true) {
    // Is there a response?
    while ((*MBOX_STATUS & MBOX_EMPTY) != 0) {
      asm volatile("nop");
    }

    // Is it a response to our message?
    if (r == *MBOX_READ) {
      // Is it a valid successful response?
      if (mbox[1] == MBOX_RESPONSE) {
        return 1;
      } else {
        return 0;
      }
    }
  }
}

void UART::init() {
  flush();

  // Disable UART0
  *registerBlock.CR = 0;

  // Set up clock for consistent divisor values
  mbox[0] = 9 * 4;
  mbox[1] = 0;
  mbox[2] = 0x38002;
  mbox[3] = 12;
  mbox[4] = 8;
  mbox[5] = 2;       // UART clock
  mbox[6] = 4000000; // 4 MHz
  mbox[7] = 0;       // Clear turbo
  mbox[8] = 0;
  call(8);

  // UART configuration
  *registerBlock.ICR = 0x7FF; // Clear pending interrupts
  *registerBlock.IBRD = 2;    // 115200 baud
  *registerBlock.FBRD = 0xB;
  *registerBlock.LCRH = 0x70; // 8n1, FIFO enabled
  *registerBlock.CR = 0x301;  // Enable UART0, Tx, Rx
}

void UART::putc(const char c) {
  while (*registerBlock.FR & 0x20) {
    // Wait for the UART to become ready to transmit.
  }
  *registerBlock.DR = c;
}

char UART::getc() {
  // TODO add in blocking mode
  while (*registerBlock.FR & 0x10) {
    CPU::nop();
  }

  const char c = *registerBlock.DR;

  // TODO add characters read/written counts
  return c;
}

void UART::flush() {
  while (*registerBlock.FR & 0x20) {
    CPU::nop();
  }
}

void UART::UartConsole::flush() {}

void UART::UartConsole::clearRx() {}

void UART::UartConsole::print(const char *format, ...) {
  va_list args;
  va_start(args, format);

  vprintf_(format, args);

  va_end(args);
}

void UART::UartConsole::printChar(char character) { uart->putc(character); }

void UART::UartConsole::printLine(const char *format, ...) {
  va_list args;
  va_start(args, format);

  vprintf_(format, args);

  uart->putc('\n');
  va_end(args);
}
char UART::UartConsole::readChar() { return uart->getc(); }
} // namespace Driver::BSP::BCM
