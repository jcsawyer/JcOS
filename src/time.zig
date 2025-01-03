const std = @import("std");
const builtin = @import("builtin");
const time = @import("std/time.zig");

const arch_time = switch (builtin.target.cpu.arch) {
    .aarch64 => @import("_arch/aarch64/time.zig"),
    else => @compileError("Unsupported architecture"),
};

var time_manager = TimeManager{};

pub fn timeManager() *TimeManager {
    return &time_manager;
}

const TimeManager = struct {
    pub fn init(_: *TimeManager) void {
        arch_time.init();
    }

    pub fn resolution(_: *TimeManager) time.Duration {
        return arch_time.resolution();
    }

    pub fn uptime(_: *TimeManager) time.Duration {
        return arch_time.uptime();
    }

    pub fn spin_for(_: *TimeManager, duration: time.Duration) void {
        arch_time.spin_for(duration);
    }
};
