const std = @import("std");
const bsp = @import("bsp.zig").rpi;
const cpu = @import("cpu.zig").boot;

comptime {
    asm (cpu);
}

pub fn panic(message: []const u8, stack_trace: ?*std.builtin.StackTrace, ret_address: ?usize) noreturn {
    _ = message;
    _ = stack_trace;
    _ = ret_address;
    while (true) {}
}
