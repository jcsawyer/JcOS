#pragma once
#include <bsp/raspberrypi/memory.hpp>
#include <stdint.h>

#if BOARD == bsp_rpi3 || BOARD == bsp_rpi4
#include <arch/aarch64/mailbox/mailbox.hpp>
#endif

namespace Mailbox::RaspberryPi {
enum Channel {
  Power = 0,
  FrameBuffer = 1,
  VirtualUART = 2,
  VCHIQ = 3,
  LEDs = 4,
  Buttons = 5,
  Touch = 6,
  Count = 7,
  Property = 8
};

class RaspberryPiMailbox : public Mailbox::Mailbox {
public:
  RaspberryPiMailbox(Channel channel) : Mailbox(), channel(channel) {}

  Response Call(Request *request) override;
  bool IsFull() override { return (*MAILBOX_STATUS & MAIL_FULL) != 0; }
  bool IsEmpty() override { return (*MAILBOX_STATUS & MAIL_EMPTY) != 0; }
  alignas(16) uint32_t Data[36];

protected:
  uint32_t MAILBOX_BASE = Memory::Map::getMMIO().VIDEOCORE_MBOX_START;
  uint32_t MAIL_FULL = 0x80000000;
  uint32_t MAIL_EMPTY = 0x40000000;
  uint32_t MAILBOX_RESPONSE = 0x80000000;

private:
  const volatile uint32_t *MAILBOX_READ =
      reinterpret_cast<volatile uint32_t *>(MAILBOX_BASE + 0x0);
  const volatile uint32_t *MAILBOX_STATUS =
      reinterpret_cast<volatile uint32_t *>(MAILBOX_BASE + 0x18);
  volatile uint32_t *MAILBOX_WRITE =
      reinterpret_cast<volatile uint32_t *>(MAILBOX_BASE + 0x20);
  Channel channel;
};
} // namespace Mailbox::RaspberryPi
