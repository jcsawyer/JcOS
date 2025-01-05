#pragma once

void spinForCycles(unsigned int cycles) {
    for (int i = 0; i < 150; i++) {
        asm volatile("nop");
    }
}