const std = @import("std");
const null_console = @import("console/null_console.zig");

pub const Console = struct {
    const Self = @This();

    context: *anyopaque,
    printFn: *const fn (context: *anyopaque, comptime str: []const u8, args: anytype) void,

    pub fn flush(_: Self) void {}

    pub fn clearRx(_: Self) void {}

    pub fn print(self: Self, comptime str: []const u8, args: anytype) void {
        return self.printFn(self.context, str, args);
    }

    pub fn printChar(_: Self, _: u8) void {}

    pub fn printLn(self: Self, comptime str: []const u8, args: anytype) void {
        self.print(str ++ "\n", args);
    }

    pub fn readChar(_: Self) u8 {
        return ' ';
    }
};

const current_console: *Console = @constCast(&@import("bsp.zig").board.console); //@constCast(&null_console.getNullConsole());

pub fn register_console(new_console: *const Console) void {
    current_console = @constCast(new_console);
}

pub fn console() Console {
    return current_console.*;
}
