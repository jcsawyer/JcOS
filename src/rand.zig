const std = @import("std");
const gpio = @import("gpio.zig");

const RNG_CTRL: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00104000);
const RNG_STATUS: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00104004);
const RNG_DATA: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00104008);
const RNG_INT_MASK: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00104010);

pub fn init() void {
    RNG_STATUS.* = 0x40000;
    // mask interrupt
    RNG_INT_MASK.* |= 1;
    // enable RNG
    RNG_CTRL.* |= 1;
}

pub fn next(min: u32, max: u32) u32 {
    while (RNG_STATUS.* >> 24 == 0) {
        asm volatile ("nop");
    }

    return RNG_DATA.* % (max - min) + min;
}
