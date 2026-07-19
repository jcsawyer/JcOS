#pragma once

#include <stdint.h>

namespace CPU {
namespace Boot {

bool processDeviceTree(uintptr_t deviceTreePhysAddr);

} // namespace Boot
} // namespace CPU
