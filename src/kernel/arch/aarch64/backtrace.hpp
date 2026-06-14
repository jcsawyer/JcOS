#pragma once

#include <backtrace.hpp>
#include <stddef.h>

namespace Backtrace::Arch {

using WalkCallback = bool (*)(const BacktraceItem &item, size_t frameIndex,
                              void *context);

void walk(WalkCallback callback, void *context);

} // namespace Backtrace::Arch
