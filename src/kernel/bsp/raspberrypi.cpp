#include <arch/aarch64/memory/registers/midr_el1.hpp>
#include <bsp/bsp.hpp>

const char *BSP::Board::DeviceName() {
  Memory::Registers::MidrEL1::print();
  return Memory::Registers::MidrEL1::PartNum();
}
