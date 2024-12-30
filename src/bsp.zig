const std = @import("std");

const use_rpi3 = std.builtin.define("bsp_rpi3");
const use_rpi4 = std.builtin.define("bsp_rpi4");

const raspi = if (use_rpi3 or use_rpi4) @import("bsp/raspberrypi.zig") else null;

pub const rpi = if (raspi != null) raspi.rpi else null;
