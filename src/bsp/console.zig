const std = @import("std");
const rpi_console = @import("raspberrypi/console.zig");

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

//const ConsoleWriter = struct {
//    const Self = @This();
//    pub const Error = error{};
//    const Writer = std.io.Writer(
//        *ConsoleWriter,
//        error{},
//        write,
//    );
//
//    pub fn write(_: *Self, bytes: []const u8) !usize {
//        rpi_console.getConsole().print(bytes);
//        return bytes.len;
//    }
//
//    pub fn writeByte(_: Self, byte: u8) !void {
//        rpi_console.getConsole().print(&.{byte});
//    }
//
//    pub fn writeByteNTimes(_: Self, byte: u8, n: usize) !void {
//        for (0..n) |_| {
//            rpi_console.getConsole().print(&.{byte});
//        }
//    }
//
//    pub fn writeBytesNTimes(_: Self, bytes: []const u8, n: usize) !void {
//        for (0..n) |_| {
//            rpi_console.getConsole().print(bytes);
//        }
//    }
//
//    pub fn writeAll(_: Self, bytes: []const u8) !void {
//        rpi_console.getConsole().print(bytes);
//    }
//
//    pub fn writer(self: *ConsoleWriter) Writer {
//        return .{ .context = self };
//    }
//};
