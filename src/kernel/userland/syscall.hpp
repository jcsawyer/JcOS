#pragma once
#include <stdint.h>

namespace Syscall {

uint64_t dispatch(uint64_t nr, uint64_t a0, uint64_t a1, uint64_t a2,
                  uint64_t a3);

} // namespace Syscall
