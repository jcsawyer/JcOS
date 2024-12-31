const std = @import("std");

pub const cpu = @import("raspberrypi/cpu.zig");
const raspi_console = @import("raspberrypi/console.zig");

pub const console = raspi_console.getConsole();
