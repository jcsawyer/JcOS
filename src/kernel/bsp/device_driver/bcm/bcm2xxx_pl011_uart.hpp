#pragma once

#include "../../../console/console.hpp"
#include "../../../driver/driver.hpp"
#include <bsp/exception/asynchronous.hpp>
#include <exceptions/asynchronous.hpp>
#include <stdint.h>
#include <synchronization.hpp>

namespace Driver::BSP::BCM {
class UART : public Driver::DeviceDriver,
             public Exceptions::Asynchronous::IRQHandler {
public:
  UART(uint32_t mmio_start_addr) : registerBlock(mmio_start_addr) {};
  const char *compatible() override { return "BCM PL011 UART"; }
  void init() override;
  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override;
  void putc(const char c) const;
  char getc() const;
  void flush() const;
  bool handle() override;
  Syncrhonization::IRQSafeNullLock<UART> lock() {
    return Syncrhonization::IRQSafeNullLock<UART>(*this);
  }
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
    volatile uint32_t *MIS;
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
      MIS = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x40);
      ICR = reinterpret_cast<volatile uint32_t *>(mmio_start_addr + 0x44);
      MBOX_READ = reinterpret_cast<volatile uint32_t *>(0xB880 + 0x00);
      MBOX_STATUS = reinterpret_cast<volatile uint32_t *>(0xB880 + 0x18);
      MBOX_WRITE = reinterpret_cast<volatile uint32_t *>(0xB880 + 0x20);
    }
  };
  RegisterBlock registerBlock;
};

static Exceptions::Asynchronous::IRQHandlerDescriptor descriptor;

static void
setDescriptor(Exceptions::Asynchronous::IRQHandlerDescriptor newDescriptor) {
  descriptor = newDescriptor;
}

static Exceptions::Asynchronous::IRQHandlerDescriptor getDescriptor() {
  return descriptor;
}

} // namespace Driver::BSP::BCM
