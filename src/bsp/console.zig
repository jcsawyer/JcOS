const std = @import("std");

pub const Console = struct {
    const Self = @This();

    context: *anyopaque,
    printFn: *const fn (context: *anyopaque, comptime str: []const u8, args: anytype) void,

    pub fn print(self: Self, comptime str: []const u8, args: anytype) void {
        return self.printFn(self.context, str, args);
    }

    pub fn printLn(self: Self, comptime str: []const u8) void {
        self.print(str);
        self.print("\n");
    }
};
