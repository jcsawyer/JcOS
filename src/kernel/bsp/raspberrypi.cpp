#include <arch/aarch64/memory/registers/midr_el1.hpp>
#include <bsp/bsp.hpp>

void BSP::Board::PrintInfo() { Memory::Registers::MidrEL1::print(); }
