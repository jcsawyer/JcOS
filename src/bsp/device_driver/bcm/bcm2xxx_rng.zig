const std = @import("std");
const DeviceDriver = @import("../../../driver.zig").DeviceDriver;
const cpu = @import("../../../_arch/aarch64/cpu.zig");

const RegisterBlock = struct {
    RNG_CTRL: *volatile u32,
    RNG_STATUS: *volatile u32,
    RNG_DATA: *volatile u32,
    RNG_INT_MASK: *volatile u32,

    pub fn new(mmio_start_addr: usize) RegisterBlock {
        return .{
            .RNG_CTRL = @ptrFromInt(mmio_start_addr + 0x00),
            .RNG_STATUS = @ptrFromInt(mmio_start_addr + 0x04),
            .RNG_DATA = @ptrFromInt(mmio_start_addr + 0x08),
            .RNG_INT_MASK = @ptrFromInt(mmio_start_addr + 0x10),
        };
    }
};

const RNGInner = struct {
    registers: RegisterBlock,

    pub fn new(mmio_start_addr: usize) RNGInner {
        return .{
            .registers = RegisterBlock.new(mmio_start_addr),
        };
    }

    pub fn init(self: *RNGInner) void {
        // Check if RNG is already enabled
        if (self.registers.RNG_CTRL.* & 1 == 1) {
            return;
        }

        self.registers.RNG_STATUS.* = 0x40000;
        // Mask interrupt
        self.registers.RNG_INT_MASK.* |= 1;
        // Enable RNG
        self.registers.RNG_CTRL.* = 1;
        // Wait for RNG to be entropy ready
        while (self.registers.RNG_STATUS.* >> 24 == 0) {
            cpu.nop();
        }
    }

    pub fn next(self: *RNGInner, min: u32, max: u32) u32 {
        while (self.registers.RNG_STATUS.* >> 24 == 0) {
            cpu.nop();
        }

        return self.registers.RNG_DATA.* % (max - min) + min;
    }
};

pub const RNG = struct {
    const Self = @This();
    inner: RNGInner,

    pub fn new(mmio_start_addr: usize) RNG {
        return RNG{
            .inner = RNGInner.new(mmio_start_addr),
        };
    }

    pub fn compatible(_: *anyopaque) []const u8 {
        return "BCM HW RNG";
    }

    pub fn init(ctx: *anyopaque) anyerror!void {
        const self: *RNG = @alignCast(@ptrCast(ctx));
        return self.inner.init();
    }

    const vtable = DeviceDriver.VTable{
        .compatible = RNG.compatible,
        .init = RNG.init,
    };

    pub fn next(self: *RNG, min: u32, max: u32) u32 {
        return self.inner.next(min, max);
    }

    pub fn driver(rng: *RNG) DeviceDriver {
        return DeviceDriver{
            .ctx = @ptrCast(rng),
            .vtable = &RNG.vtable,
        };
    }
};
