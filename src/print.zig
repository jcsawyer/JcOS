const std = @import("std");
const console = @import("console.zig").console;
const time = @import("time.zig").timeManager;

pub extern fn printf_(format: [*:0]const u8, ...) c_int;

export fn _putchar(c: i32) void {
    console().printChar(@as(u8, @intCast(c)));
}

fn call_printf(comptime fmt: [*:0]const u8, args: anytype) void {
    _ = @call(std.builtin.CallModifier.auto, printf_, .{fmt} ++ args);
}

pub fn print(comptime str: []const u8, args: anytype) void {
    call_printf(@as([*:0]const u8, @ptrCast(str)), args);
}

pub fn printChar(c: u8) void {
    console().printChar(c);
}

pub fn info(comptime str: []const u8, args: anytype) void {
    const uptime = time().uptime();
    call_printf("[ %03d.%0-.6d ] ", .{ uptime.asSecs(), uptime.asMicros() });
    call_printf(@as([*:0]const u8, @ptrCast(str)), args);
    call_printf("\n", .{});
}

pub fn warn(comptime str: []const u8, args: anytype) void {
    const uptime = time().uptime();
    call_printf("[W%03d.%0-.6d ] ", .{ uptime.asSecs(), uptime.asMicros() });
    call_printf(@as([*:0]const u8, @ptrCast(str)), args);
    call_printf("\n", .{});
}

pub fn panic(comptime str: []const u8, args: anytype) void {
    const uptime = time().uptime();
    call_printf("[P%03d.%0-.6d ] ", .{ uptime.asSecs(), uptime.asMicros() });
    call_printf(@as([*:0]const u8, @ptrCast(str)), args);
    call_printf("\n", .{});
}

pub fn readChar() u8 {
    return console().readChar();
}

pub fn clearRx() void {
    console().clearRx();
}
