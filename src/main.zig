const std = @import("std");
const bsp = @import("bsp.zig").raspi;
const cpu = @import("cpu.zig");

comptime {
    asm (cpu.arch_boot());
    // Need to ensure the bsp cpu values are not compiled away
    _ = bsp.cpu.BOOT_CORE_ID;
}

pub fn kernel_init() noreturn {
    @panic("Kernel initialization is not implemented");
}

pub fn panic(message: []const u8, stack_trace: ?*std.builtin.StackTrace, ret_address: ?usize) noreturn {
    _ = message;
    _ = stack_trace;
    _ = ret_address;

    cpu.cpu.wait_forever();
}
