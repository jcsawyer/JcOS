const gpio = @import("gpio.zig");

const SYSTMR_LO: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00003004);
const SYSTMR_HI: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00003008);

pub fn wait_cycles(count: u32) void {
    if (count != 0) {
        var c = count;
        while (c != 1) : (c -= 1) {
            asm volatile ("nop");
        }
    }
}

pub fn wait_msec(count: u32) void {
    var f: u64 = undefined;
    var t: u64 = undefined;
    var r: u64 = undefined;

    // Get the current counter frequency
    asm volatile ("mrs %[freq], cntfrq_el0"
        : [freq] "=r" (f),
    );

    // Read the current counter value
    asm volatile ("mrs %[time], cntpct_el0"
        : [time] "=r" (t),
    );

    // Calculate the required counter increase
    const i: u64 = ((f / 1000) * count) / 1000;

    // Loop until the counter increase is greater than or equal to `i`
    while (true) {
        asm volatile ("mrs %[curr], cntpct_el0"
            : [curr] "=r" (r),
        );
        if (r - t >= i) {
            break;
        }
    }
}

pub fn get_system_timer() u64 {
    var h: u32 = 0;
    var l: u32 = undefined;

    h = SYSTMR_HI.*;
    l = SYSTMR_LO.*;

    if (h != SYSTMR_HI.*) {
        h = SYSTMR_HI.*;
        l = SYSTMR_LO.*;
    }
    // compose long int value
    const bit_shift: u32 = 32;
    return @as(u64, (@as(u64, h) << bit_shift) | l);
}

pub fn wait_msec_st(count: u32) void {
    const t: u64 = get_system_timer();
    // we myst check if it's non-zero because qemu does not emulator
    // system timer, and returning constant zero would mean infite loop
    if (t != 0) {
        while (get_system_timer() - t < count) {}
    }
}
