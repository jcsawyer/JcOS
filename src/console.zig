const std = @import("std");
const null_console = @import("console/null_console.zig");

pub const Console = struct {
    const Self = @This();

    context: *anyopaque,
    vtable: *const VTable,

    pub const VTable = struct {
        flush: *const fn (context: *anyopaque) void,
        clearRx: *const fn (context: *anyopaque) void,
        print: *const fn (context: *anyopaque, comptime str: []const u8, args: anytype) void,
        printChar: *const fn (context: *anyopaque, char: u8) void,
        readChar: *const fn (context: *anyopaque) u8,
    };

    pub fn flush(self: Self) void {
        return self.vtable.flush(self.context);
    }

    pub fn clearRx(self: Self) void {
        return self.vtable.clearRx(self.context);
    }

    pub fn print(self: Self, comptime str: []const u8, args: anytype) void {
        return self.vtable.print(self.context, str, args);
    }

    pub fn printChar(self: Self, char: u8) void {
        return self.vtable.printChar(self.context, char);
    }

    pub fn printLn(self: Self, comptime str: []const u8, args: anytype) void {
        return self.vtable.print(self.context, str ++ "\n", args);
    }

    pub fn readChar(self: Self) u8 {
        return self.vtable.readChar(self.context);
    }
};

const current_console: *Console = @constCast(&@import("bsp.zig").board.console); //@constCast(&null_console.getNullConsole());

pub fn register_console(new_console: *const Console) void {
    current_console = @constCast(new_console);
}

pub fn console() Console {
    return current_console.*;
}
