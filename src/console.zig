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

    pub fn printChar(self: Self, char: u8) void {
        self.printFn(&.{char}, void);
    }

    pub fn printLn(self: Self, comptime str: []const u8, args: anytype) void {
        self.print(str ++ "\n", args);
    }
};

const current_console: *const Console = &@import("bsp.zig").board.console; //&null_console.getNullConsole();

pub fn register_console(new_console: *const Console) void {
    current_console = new_console;
}

pub fn console() Console {
    return current_console.*;
}
