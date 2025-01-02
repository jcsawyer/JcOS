const console = @import("console.zig").console;

pub fn print(comptime str: []const u8, args: anytype) void {
    console().print(str, args);
}

pub fn printLn(comptime str: []const u8, args: anytype) void {
    console().printLn(str, args);
}

pub fn printChar(char: u8) void {
    console().printChar(char);
}

pub fn readChar() u8 {
    return console().readChar();
}

pub fn clearRx() void {
    console().clearRx();
}
