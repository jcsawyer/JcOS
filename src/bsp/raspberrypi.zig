const std = @import("std");
const config = @import("config");

pub const cpu = @import("raspberrypi/cpu.zig");
const raspi_console = @import("raspberrypi/console.zig");
const raspi_drivers = @import("raspberrypi/driver.zig");
const uart_console = @import("device_driver/bcm/bcm2xxx_pl011_uart.zig");

pub fn board_name() []const u8 {
    if (std.mem.eql(u8, config.board, "bsp_rpi3")) {
        return "Raspberry Pi 3";
    } else if (std.mem.eql(u8, config.board, "bsp_rpi4")) {
        return "Raspberry Pi 4";
    } else {
        return "Unknown";
    }
}

pub const console = uart_console.getConsole();
pub const driver = raspi_drivers;
