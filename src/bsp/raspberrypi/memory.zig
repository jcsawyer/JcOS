const std = @import("std");
const config = @import("config");

pub const MMIO = struct {
    START: usize,
    GPIO_START: usize,
    PL011_UART_START: usize,
};

pub const map = struct {
    pub const BOARD_DEFAULT_LOAD_ADDRESS: usize = 0x80000;

    pub const GPIO_OFFSET: usize = 0x0020_0000;
    pub const UART_OFFSET: usize = 0x0020_1000;

    pub const mmio = blk: {
        if (std.mem.eql(u8, config.board, "bsp_rpi3")) {
            break :blk MMIO{
                .START = 0x3F00_0000,
                .GPIO_START = 0x3F00_0000 + GPIO_OFFSET,
                .PL011_UART_START = 0x3F00_0000 + UART_OFFSET,
            };
        } else if (std.mem.eql(u8, config.board, "bsp_rpi4")) {
            break :blk MMIO{
                .START = 0xFE00_0000,
                .GPIO_START = 0xFE00_0000 + GPIO_OFFSET,
                .PL011_UART_START = 0xFE00_0000 + UART_OFFSET,
            };
        } else {
            @panic("Cannot map pl011 uart for unknown board");
        }
    };
};

/// The address on which the Raspberry firmware loads every binary by default.
pub fn boardDefaultLoadAddr() *const u64 {
    return @ptrFromInt(map.BOARD_DEFAULT_LOAD_ADDRESS);
}
