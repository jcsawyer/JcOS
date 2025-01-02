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

    fn printChar(_: *anyopaque, char: u8) void {
        const uart: (*volatile u8) = @ptrFromInt(0x3F201000);
        if (char == '\n') {
            uart.* = '\r';
        }

        uart.* = char;
    }

    fn readChar(_: Self) u8 {
        const uart: (*volatile u8) = @ptrFromInt(0x3F201000);
        return uart.*;
    }

    const vtable = bsp_console.VTable{
        .flush = QEMUConsole.flush,
        .clearRx = QEMUConsole.clearRx,
        .print = QEMUConsole.print,
        .printChar = QEMUConsole.printChar,
        .readChar = QEMUConsole.readChar,
    };

    fn console(self: *QEMUConsole) bsp_console {
        return .{
            .context = self,
            .vtable = &QEMUConsole.vtable,
        };
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
