#include "main.hpp"

#include <stdint.h>

namespace {

void prepareEl2ToEl1Transition(uint64_t physBootCoreStackEndExclusiveAddress) {
  asm volatile("msr CNTHCTL_EL2, %[value]" : : [value] "r"(0b11));
  asm volatile("msr CNTVOFF_EL2, %[value]" : : [value] "r"(0));
  asm volatile("msr HCR_EL2, %[value]" : : [value] "r"(0b1ull << 31));

  const uint64_t spsrValue =
      (0b1ull << 9) | (0b1ull << 8) | (0b1ull << 7) | (0b1ull << 6) | 0b0101;
  asm volatile("msr SPSR_EL2, %[value]" : : [value] "r"(spsrValue));
  asm volatile("msr ELR_EL2, %[value]"
               :
               : [value] "r"(reinterpret_cast<uint64_t>(&chainloader_init)));
  asm volatile("msr SP_EL1, %[value]"
               :
               : [value] "r"(physBootCoreStackEndExclusiveAddress));
}

} // namespace

extern "C" void _start_cpp(uint64_t physBootCoreStackEndExclusiveAddress) {
  prepareEl2ToEl1Transition(physBootCoreStackEndExclusiveAddress);
  asm volatile("eret");
}
