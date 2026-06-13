#include <stdint.h>

extern "C" {
__attribute__((section(".text._start_arguments"), used,
               visibility("default"))) extern uint64_t
    BOOT_CORE_ID asm("BOOT_CORE_ID") = 0;
}
