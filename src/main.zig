const std = @import("std");
const cpu = @import("cpu.zig");
const bsp = @import("bsp.zig");
comptime {
    asm (cpu.arch_boot());
    // Need to ensure the bsp cpu values are not compiled away
    std.mem.doNotOptimizeAway(bsp.board.cpu.BOOT_CORE_ID);
}
