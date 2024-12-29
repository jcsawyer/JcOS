const gpio = @import("gpio.zig");
const mbox = @import("mbox.zig");
const delays = @import("delays.zig");

const PM_RSTC: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x0010001c);
const PM_RSTS: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00100020);
const PM_WDOG: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00100024);
const PM_WDOG_MAGIC: u32 = 0x5a000000;
const PM_RSTC_FULLRST: u32 = 0x00000020;

pub fn off() void {
    var r: u64 = undefined;
    // power off devices one by one
    for (0..16) |i| {
        mbox.mbox[0] = 8 * 4;
        mbox.mbox[1] = mbox.MBOX_REQUEST;
        mbox.mbox[2] = mbox.MBOX_TAG_SETPOWER;
        mbox.mbox[3] = 8;
        mbox.mbox[4] = 8;
        mbox.mbox[5] = @as(u32, @intCast(i));
        mbox.mbox[7] = mbox.MBOX_TAG_LAST;
        _ = mbox.call(mbox.MBOX_CH_PROP);
        r += 1;
    }

    gpio.GPFSEL0.* = 0;
    gpio.GPFSEL1.* = 0;
    gpio.GPFSEL2.* = 0;
    gpio.GPFSEL3.* = 0;
    gpio.GPFSEL4.* = 0;
    gpio.GPFSEL5.* = 0;

    gpio.GPPUD.* = 0;
    delays.wait_cycles(150);
    gpio.GPPUDCLK0.* = 0xffffffff;
    gpio.GPPUDCLK1.* = 0xffffffff;
    delays.wait_cycles(150);
    gpio.GPPUDCLK0.* = 0;
    gpio.GPPUDCLK1.* = 0; // flush GPIO setup

    // power off the SoC (GPU + CPU)
    r = PM_RSTS.*;
    const mask: u32 = 0xfffffaaa;
    r &= ~mask;
    r |= 0x555; // partition 63 used to indicate halt
    PM_RSTS.* = PM_WDOG_MAGIC | @as(u32, @intCast(r));
    PM_WDOG.* = PM_WDOG_MAGIC | 10;
    PM_RSTC.* = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
}

pub fn reset() void {
    var r: u32 = undefined;
    // trigger restart by instructing the GPU to boot from parition 0
    r = PM_RSTS.*;
    const mask: u32 = 0xfffffaaa;
    r &= ~mask;
    PM_RSTS.* = PM_WDOG_MAGIC | @as(u32, @intCast(r));
    PM_WDOG.* = PM_WDOG_MAGIC | 10;
    PM_RSTC.* = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
}
