const std = @import("std");
const kernel_init = @import("../../../main.zig").kernel_init;

const CONST_CORE_ID_MASK: usize = 0b11;

pub fn boot_asm() []u8 {
    var buff: [2048]u8 = undefined;
    const asm_code = @embedFile("boot.s");
    @setEvalBranchQuota(10000);
    _ = std.mem.replace(u8, asm_code, "{CONST_CORE_ID_MASK}", "0b11", buff[0..]);
    return buff[0..];
}

export fn _start_zig() noreturn {
    kernel_init();
}
