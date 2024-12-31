const std = @import("std");
const builtin = @import("builtin");

pub const arch_boot = switch (builtin.target.cpu.arch) {
    .aarch64 => @import("../_arch/aarch64/cpu/boot.zig"),
    else => @compileError("Unsupported architecture"),
}.boot_asm;

pub const cpu = switch (builtin.target.cpu.arch) {
    .aarch64 => @import("../_arch/aarch64/cpu.zig"),
    else => @compileError("Unsupported architecture"),
};
