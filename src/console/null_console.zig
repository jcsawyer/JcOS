const std = @import("std");
const bsp_console = @import("../console.zig").Console;

var nullConsole = NullConsole{};

pub fn getNullConsole() bsp_console {
    return nullConsole.console();
}

pub const NullConsole = struct {
    const Self = @This();

    fn flush(_: *NullConsole) void {}

    fn clearRx(_: *NullConsole) void {}

    fn print(_: *anyopaque, comptime _: []const u8, _: anytype) void {
        // Do nothing
    }

    fn console(self: *NullConsole) bsp_console {
        return .{ .context = self, .printFn = print };
    }
};
