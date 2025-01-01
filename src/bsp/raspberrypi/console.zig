const std = @import("std");
const bsp_console = @import("../../console.zig").Console;

var console = QEMUConsole{};

pub fn getConsole() bsp_console {
    return console.console();
}

pub const QEMUConsole = struct {
    const Self = @This();

    fn flush(_: *QEMUConsole) void {}

    fn clearRx(_: *QEMUConsole) void {}

    fn print(_: *anyopaque, comptime str: []const u8, args: anytype) void {
        std.fmt.format(ConsoleWriter{ .context = {} }, str, args) catch unreachable;
    }

    fn console(self: *QEMUConsole) bsp_console {
        return .{ .context = self, .printFn = print };
    }
};

const Context = void;
const WriteError = anyerror;
fn writeFn(_: Context, buf: []const u8) anyerror!usize {
    const uart: (*volatile u8) = @ptrFromInt(0x3F201000);
    for (buf) |char| {
        if (char == '\n') {
            uart.* = '\r';
        }

        uart.* = char;
    }
    return buf.len;
}
const ConsoleWriter = std.io.GenericWriter(Context, WriteError, writeFn);
