#include "main.hpp"

#include <stdint.h>

extern "C" void _start_cpp(uint64_t physBootCoreStackEndExclusiveAddress,
                           uintptr_t deviceTreePhysAddr) {
  (void)physBootCoreStackEndExclusiveAddress;
  chainloader_init(deviceTreePhysAddr);
}
