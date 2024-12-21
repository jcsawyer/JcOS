const std = @import("std");
const AtomicOrder = std.builtin.AtomicOrder;

const MMIO_BASE: u32 = 0x3F000000;

pub fn write(reg: usize, data: u32) void {
    @fence(AtomicOrder.seq_cst);
    const ptr: *volatile u32 = @ptrFromInt(reg);
    ptr.* = data;
}

pub fn read(reg: usize) u32 {
    @fence(AtomicOrder.seq_cst);
    const ptr: *volatile u32 = @ptrFromInt(reg);
    return ptr.*;
}
