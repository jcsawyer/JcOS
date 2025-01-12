#pragma once
#include <memory/memory_register.hpp>
#include <print.hpp>

namespace Memory::Registers {

class MidrEL1 : public Memory::InMemoryRegister<MidrEL1> {
public:
  static uint64_t get() {
    uint64_t value = 0;
    asm volatile("mrs %0, midr_el1" : "=r"(value));
    return value;
  };

  static void print() {
    info("Identifying the device...");
    uint64_t midr_el1 = get();
    info("MIDR_EL1: 0x%08lX", midr_el1);
    info("      Implementer:  0x%03X [ %s ]", (midr_el1 >> 24) & 0xFF,
         formatImplementer());
    info("      Variant:      0x%03X", (midr_el1 >> 20) & 0xF);
    info("      Architecture: 0x%03X [ %s ]", (midr_el1 >> 16) & 0xF,
         formatArchitecture());
    info("      PartNum:      0x%03X [ %s ]", (midr_el1 >> 4) & 0xFFF,
         PartNum());
    info("      Revision:     0x%03X", midr_el1 & 0xF);
  }

  static const char *PartNum() {
#if BOARD == bsp_rpi3 || BOARD == bsp_rpi4
    uint64_t value = (get() >> 4) & 0xFFF;
    switch (value) {
    case 0xB76:
      return "Raspberry Pi";
    case 0xC07:
      return "Raspberry Pi 2";
    case 0xD03:
      return "Raspberry Pi 3";
    case 0xD08:
      return "Raspberry Pi 4";
    }
#else
    return "Unknown";
#endif
  }

private:
  static const char *formatImplementer() {
    uint64_t value = (get() >> 24) & 0xFF;
    switch (value) {
    case 0x00:
      return "Reserved for software use";
    case 0xC0:
      return "Ampere Computing";
    case 0x41:
      return "Arm Limited";
    case 0x42:
      return "Broadcom Corporation";
    case 0x43:
      return "Cavium Inc.";
    case 0x44:
      return "Digital Equipment Corporation";
    case 0x46:
      return "Fujitsu Ltd.";
    case 0x49:
      return "Infineon Technologies AG";
    case 0x4D:
      return "Motorola or Freescale Semiconductor Inc.";
    case 0x4E:
      return "NVIDIA Corporation";
    case 0x50:
      return "Applied Micro Circuits Corporation";
    case 0x51:
      return "Qualcomm Inc.";
    case 0x56:
      return "Marvell International Ltd.";
    case 0x69:
      return "Intel Corporation";
    default:
      return "Unknown";
    }
  }

  static const char *formatArchitecture() {
    uint64_t value = (get() >> 16) & 0xF;
    switch (value) {
    case 0b0001:
      return "Armv4";
    case 0b0010:
      return "Armv4T";
    case 0b0011:
      return "Armv5 (obsolete)";
    case 0b0100:
      return "Armv5T";
    case 0b0101:
      return "Armv5TE";
    case 0b0110:
      return "Armv5TEJ";
    case 0b0111:
      return "Armv6";
    case 0b1111:
      return "Individually Defined";
    default:
      return "Unknown";
    }
  }
};

} // namespace Memory::Registers