const std = @import("std");
const cpu = @import("cpu.zig");
const bsp = @import("bsp.zig");
const console = @import("print.zig");
const driver = @import("driver.zig");
const time_manager = @import("time.zig").timeManager;
const time = @import("std/time.zig");

comptime {
    asm (cpu.arch_boot());
    // Need to ensure the bsp cpu values are not compiled away
    std.mem.doNotOptimizeAway(bsp.board.cpu.BOOT_CORE_ID);
}

pub fn kernel_init() noreturn {
    time_manager().init();
    bsp.board.driver.init() catch {
        @panic("Error loading drivers");
    };

    driver.driver_manager().init() catch {
        @panic("Error initializing drivers");
    };

    kernel_main();
}

pub fn kernel_main() noreturn {
    console.print("%s", .{@as([*:0]const u8, @ptrCast(logo))});
    console.info("Hello, %s!", .{"World"});
    console.info("%s version %s", .{ "JcOS", "0.1.0" });
    console.info("Booting on: %s", .{@as([*:0]const u8, @ptrCast(bsp.board.board_name()))});
    console.info("Architectural timer resoltion: %d", .{time_manager().resolution().asNanos()});
    console.info("Drivers loaded:", .{});
    driver.driver_manager().print_drivers();

    while (true) {
        const c = console.readChar();
        console.printChar(c);
        //console.info("Spinning for 1 second", .{});
        //time_manager().spin_for(time.Duration.fromSecs(1));
    }
    @panic("Kernel initialization is not implemented");
}

var already_panicking: bool = false;
pub fn panic(message: []const u8, stack_trace: ?*std.builtin.StackTrace, ret_address: ?usize) noreturn {
    _ = stack_trace;
    @setCold(true);
    if (already_panicking) {
        console.panic("\nPANIC in PANIC", .{});
    }
    already_panicking = true;

    console.panic("KERNEL PANIC: (ret: 0x%08X)", .{ret_address orelse 0});
    console.panic("\t%s", .{@as([*:0]const u8, @ptrCast(message))});
    cpu.cpu.wait_forever();
}

const logo: []const u8 =
    \\                       
    \\    __     _____ _____ 
    \\ __|  |___|     |   __|
    \\|  |  |  _|  |  |__   |
    \\|_____|___|_____|_____|
    \\         Version 0.1.0
    \\
    \\
;
