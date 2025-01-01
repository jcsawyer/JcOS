const std = @import("std");
const config = @import("config");
const cpu = @import("../../../cpu.zig");

const RegisterBlock = struct {
    DR: *volatile u32,
    reserved1: u32,
    FR: *volatile u32,
    reserved2: u32,
    IBDR: *volatile u32,
    FBRD: *volatile u32,
    LCRH: *volatile u32,
    CR: *volatile u32,
    reserved3: u32,
    ICTLR: *volatile u32,
};

const MMIODerefWarpper = struct {
    registers: *const volatile RegisterBlock,

    pub fn new(mmio_start_addr: usize) MMIODerefWarpper {
        return .{
            .registers = @ptrCast(&mmio_start_addr),
        };
    }
};

const PL011UartInner = struct {
    registers: *const volatile RegisterBlock,

    pub fn new(mmio_start_addr: usize) PL011UartInner {
        return .{
            .registers = @ptrCast(&mmio_start_addr),
        };
    }

    pub fn init(_: *PL011UartInner) void {
        @import("../../../bsp.zig").board.console.printLn("PL011UartInner init", .{});
    }
};

pub const PL011Uart = struct {
    const Self = @This();
    inner: PL011UartInner,

    pub fn new(mmio_start_addr: usize) PL011Uart {
        return .{
            .inner = PL011UartInner.new(mmio_start_addr),
        };
    }

    pub fn compatible() []const u8 {
        return "PL011 UART";
    }

    pub fn init(self: *Self) anyerror!void {
        return self.inner.init();
    }
};
