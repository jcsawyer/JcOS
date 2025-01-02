const std = @import("std");
const cpu = @import("cpu.zig");
const bsp = @import("bsp.zig");
const console = @import("print.zig");
const driver = @import("driver.zig");

comptime {
    asm (cpu.arch_boot());
    // Need to ensure the bsp cpu values are not compiled away
    std.mem.doNotOptimizeAway(bsp.board.cpu.BOOT_CORE_ID);
}

pub fn kernel_init() noreturn {
    bsp.board.driver.init() catch {
        @panic("Error loading drivers");
    };

    driver.driver_manager().init() catch {
        @panic("Error initializing drivers");
    };

    kernel_main();
}

pub fn kernel_main() noreturn {
    console.printLn("[0] {s} version {s}", .{ "JcOS", "0.1.0" });
    console.printLn("[1] Booting on: {s}", .{bsp.board.board_name()});

    console.printLn("[2] Drivers loaded:", .{});
    driver.driver_manager().print_drivers();

    // Discard any spurious received characters before going into echo mode.
    console.clearRx();
    while (true) {
        const c = console.readChar();
        console.printChar(c);
    }

    @panic("Kernel initialization is not implemented");
}

var already_panicking: bool = false;
pub fn panic(message: []const u8, stack_trace: ?*std.builtin.StackTrace, ret_address: ?usize) noreturn {
    _ = stack_trace;
    @setCold(true);
    if (already_panicking) {
        console.print("\nPANIC in PANIC", .{});
    }
    already_panicking = true;

    console.print("\n\nKERNEL PANIC: (ret: {?X})\n\t{s}\n\n", .{ ret_address, message });
    cpu.cpu.wait_forever();
}
