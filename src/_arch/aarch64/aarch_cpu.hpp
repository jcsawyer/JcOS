#pragma once

void spinForCycles(unsigned int cycles) {
    for (unsigned int i = 0; i < cycles; i++) {
        asm volatile("nop");
    }
}
