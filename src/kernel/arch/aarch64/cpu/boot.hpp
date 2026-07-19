#pragma once

#include <stdint.h>
#include <stddef.h>

namespace CPU {
namespace Boot {

constexpr size_t MAX_CORES = 16;

using ReleasedCoreWork = void (*)(size_t coreId);

bool processDeviceTree(uintptr_t deviceTreePhysAddr);
bool releaseCore(size_t coreId);
bool isCoreReleased(size_t coreId);
void setReleasedCoreWork(ReleasedCoreWork work);
ReleasedCoreWork releasedCoreWork();

} // namespace Boot
} // namespace CPU
