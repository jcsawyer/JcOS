const std = @import("std");
const config = @import("config");

const use_rpi3 = std.mem.eql(u8, config.board, "bsp_rpi3");
const use_rpi4 = std.mem.eql(u8, config.board, "bsp_rpi4");

pub const raspi = if (use_rpi3 or use_rpi4) @import("bsp/raspberrypi.zig") else null;
