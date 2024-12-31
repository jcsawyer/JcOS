const std = @import("std");
const cpu = @import("cpu.zig");
const bsp = @import("bsp.zig").raspi;
const console = bsp.console;

comptime {
    asm (cpu.arch_boot());
    // Need to ensure the bsp cpu values are not compiled away
    _ = bsp.cpu.BOOT_CORE_ID;
}

pub fn kernel_init() noreturn {
    console.print("Hello, {s}!", .{"World"});
    @panic("Kernel initialization is not implemented");
}

var already_panicking: bool = false;
pub fn panic(message: []const u8, stack_trace: ?*std.builtin.StackTrace, ret_address: ?usize) noreturn {
    _ = stack_trace;
    _ = ret_address;
    @setCold(true);
    if (already_panicking) {
        console.print("\nPANIC in PANIC", .{});
    }
    already_panicking = true;

    console.print("\n\nKERNEL PANIC: {s}", .{message});
    cpu.cpu.wait_forever();
}
