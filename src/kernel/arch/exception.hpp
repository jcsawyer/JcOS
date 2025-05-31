#pragma once

namespace Exception {

namespace Asynchronous {

void localIrqMask();
void localIrqUnmask();
unsigned long localIrqMaskSave();
bool isLocalIrqMasked();
void localIrqRestore(unsigned long flags);

} // namespace Asynchronous

} // namespace Exception