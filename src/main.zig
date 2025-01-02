const std = @import("std");
const cpu = @import("cpu.zig");
const bsp = @import("bsp.zig");
const console = @import("print.zig");
const driver = @import("driver.zig");
const time = @import("time.zig").timeManager;
comptime {
    asm (cpu.arch_boot());
    // Need to ensure the bsp cpu values are not compiled away
    std.mem.doNotOptimizeAway(bsp.board.cpu.BOOT_CORE_ID);
}

pub fn kernel_init() noreturn {
    time().init();
    bsp.board.driver.init() catch {
        @panic("Error loading drivers");
    };

    driver.driver_manager().init() catch {
        @panic("Error initializing drivers");
    };

    kernel_main();
}

pub fn kernel_main() noreturn {
    console.info("{s} version {s}", .{ "JcOS", "0.1.0" });
    console.info("Booting on: {s}", .{bsp.board.board_name()});
    console.info("Drivers loaded:", .{});
    driver.driver_manager().print_drivers();

    // Test a failing timer case
    //time().spin_for(@import("std/time.zig").Duration.fromNanos(1));

    @panic("Kernel initialization is not implemented");
}

var already_panicking: bool = false;
pub fn panic(message: []const u8, stack_trace: ?*std.builtin.StackTrace, ret_address: ?usize) noreturn {
    _ = stack_trace;
    @setCold(true);
    if (already_panicking) {
        console.warn("\nPANIC in PANIC", .{});
    }
    already_panicking = true;

    console.panic("KERNEL PANIC: (ret: {?X})\t:\t{s}\n\n", .{ ret_address, message });
    cpu.cpu.wait_forever();
}
