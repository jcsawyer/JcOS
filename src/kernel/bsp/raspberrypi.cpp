#include <arch/aarch64/mailbox/mailbox.hpp>
#include <arch/aarch64/memory/registers/midr_el1.hpp>
#include <bsp/bsp.hpp>
#include <bsp/raspberrypi/mailbox/mailbox.hpp>
#include <bsp/raspberrypi/mailbox/messages/board_model.hpp>
#include <bsp/raspberrypi/mailbox/messages/board_revision.hpp>
#include <bsp/raspberrypi/mailbox/messages/board_serial.hpp>

static const char *formatMemorySize(uint32_t revision) {
  switch ((revision >> 20) & 0x7) {
  case 0:
    return "256MB";
  case 1:
    return "512MB";
  case 2:
    return "1GB";
  case 3:
    return "2GB";
  case 4:
    return "4GB";
  case 5:
    return "8GB";
  default:
    return "Unknown";
  }
}

static const char *formatManufacturer(uint32_t revision) {
  switch ((revision >> 16) & 0xF) {
  case 0:
    return "Sony UK";
  case 1:
    return "Egoman";
  case 2:
    return "Embest";
  case 3:
    return "Sony Japan";
  case 4:
    return "Embest";
  case 5:
    return "Stadium";
  default:
    return "Unknown";
  }
}

static const char *formatProcessor(uint32_t revision) {
  switch ((revision >> 12) & 0xF) {
  case 0:
    return "BCM2835";
  case 1:
    return "BCM2836";
  case 2:
    return "BCM2837";
  case 3:
    return "BCM2711";
  case 4:
    return "BCM2838";
  default:
    return "Unknown";
  }
}

static const char *formatType(uint32_t revision) {
  switch ((revision >> 4) & 0xFF) {
  case 0x0:
    return "A";
  case 0x1:
    return "B";
  case 0x2:
    return "A+";
  case 0x3:
    return "B+";
  case 0x4:
    return "2B";
  case 0x5:
    return "Alpha";
  case 0x6:
    return "CM1";
  case 0x8:
    return "3B";
  case 0x9:
    return "Zero";
  case 0xA:
    return "CM3";
  case 0xC:
    return "Zero W";
  case 0xD:
    return "3B+";
  case 0xE:
    return "3A+";
  case 0xF:
    return "Internal use only";
  case 0x10:
    return "CM3+";
  case 0x11:
    return "4B";
  case 0x12:
    return "Zero 2 W";
  case 0x13:
    return "400";
  case 0x14:
    return "CM4";
  case 0x15:
    return "CM4S";
  case 0x17:
    return "5";
  case 0x18:
    return "CM5";
  case 0x19:
    return "500";
  default:
    return "";
  }
}

void BSP::Board::PrintInfo() {
  Memory::Registers::MidrEL1::print();

  Mailbox::RaspberryPi::RaspberryPiMailbox mailbox(
      Mailbox::RaspberryPi::Channel::Property);

  Mailbox::RaspberryPi::Messages::BoardSerialRequest boardSerialRequset;
  Mailbox::RaspberryPi::Messages::BoardSerialResponse *boardSerialResponse =
      nullptr;
  if (!mailbox.Call(&boardSerialRequset, boardSerialResponse)) {
    warn("Failed to get board serial");
    return;
  }

  info("      Board serial:    %08X%08X",
       boardSerialResponse->BoardSerial().High,
       boardSerialResponse->BoardSerial().Low);

  Mailbox::RaspberryPi::Messages::BoardRevisionRequest boardRevisionRequest;
  Mailbox::RaspberryPi::Messages::BoardRevisionResponse *boardRevisionResponse =
      nullptr;
  if (!mailbox.Call(&boardRevisionRequest, boardRevisionResponse)) {
    warn("Failed to get board revision");
    return;
  }

  Mailbox::RaspberryPi::Messages::BoardRevisionResponse *boardRevision;
  boardRevision =
      static_cast<Mailbox::RaspberryPi::Messages::BoardRevisionResponse *>(
          boardRevisionResponse);
  uint32_t revision = boardRevision->BoardRevision();
  info("Board revision: 0x%X", revision);

  info("      Overvoltage:     %s",
       (revision >> 31 == 0) ? "Allowed" : "Disallowed");
  info("      OTP Programming: %s",
       (revision >> 30 == 0) ? "Allowed" : "Disallowed");
  info("      OTP Read:        %s",
       (revision >> 29 == 0) ? "Allowed" : "Disallowed");
  info("      Warranty:        %s",
       (revision >> 25 == 0) ? "Intact" : "Voided");
  info("      Memory Size:     %s", formatMemorySize(revision));
  info("      Manufacturer:    %s", formatManufacturer(revision));
  info("      Processor:       %s", formatProcessor(revision));
  info("      Type:            %s", formatType(revision));
  info("      Revision:        1.%d", revision & 0xF);
}
