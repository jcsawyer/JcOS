#include "../../std/stdint.h"

extern "C" {
// Explicitly name the symbol "BOOT_CORE_ID" for use in assembly.
__attribute__((section(".text._start_arguments"), used,
               visibility("default"))) extern uint64_t
    BOOT_CORE_ID asm("BOOT_CORE_ID") = 0;
}
