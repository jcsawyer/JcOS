#pragma once

#include <stddef.h>
#include <stdint.h>

namespace CPU {
namespace Boot {
namespace DTB {

uintptr_t deviceTreeAddress();
size_t cpuCoreCount();
bool processDeviceTree(uintptr_t deviceTreePhysAddr);

} // namespace DTB
} // namespace Boot
} // namespace CPU
