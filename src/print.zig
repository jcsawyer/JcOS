const console = @import("console.zig").console;
const time = @import("time.zig").timeManager;

pub fn print(comptime str: []const u8, args: anytype) void {
    console().print(str, args);
}

pub fn printLn(comptime str: []const u8, args: anytype) void {
    console().printLn(str, args);
}

pub fn printChar(char: u8) void {
    console().printChar(char);
}

pub fn info(comptime str: []const u8, args: anytype) void {
    const uptime = time().uptime();
    print("[ {d:>3}.{d:0<6} ] ", .{ uptime.asSecs(), uptime.asMicros() });
    print(str, args);
    printChar('\n');
}

pub fn warn(comptime str: []const u8, args: anytype) void {
    const uptime = time().uptime();
    print("[W{d:>3}.{d:0<6} ] ", .{ uptime.asSecs(), uptime.asMicros() });
    print(str, args);
    printChar('\n');
}

pub fn panic(comptime str: []const u8, args: anytype) void {
    const uptime = time().uptime();
    print("[P{d:>3}.{d:0<6} ] ", .{ uptime.asSecs(), uptime.asMicros() });
    print(str, args);
    printChar('\n');
}

pub fn readChar() u8 {
    return console().readChar();
}

pub fn clearRx() void {
    console().clearRx();
}
