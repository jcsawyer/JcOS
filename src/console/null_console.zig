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

    fn printChar(_: *anyopaque, _: u8) void {
        // Do nothing
    }

    pub fn readChar(_: Self) u8 {
        return ' ';
    }

    const vtable = bsp_console.VTable{
        .flush = NullConsole.flush,
        .clearRx = NullConsole.clearRx,
        .print = NullConsole.print,
        .printChar = NullConsole.printChar,
        .readChar = NullConsole.readChar,
    };

    fn console(self: *NullConsole) bsp_console {
        return .{
            .context = self,
            .vtable = &NullConsole.vtable,
        };
    }
};
