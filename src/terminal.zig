const std = @import("std");
const fonts = @import("fonts.zig");
const FrameBuffer = @import("framebuffer.zig").FrameBuffer;
const color = @import("framebuffer.zig").Color;

var fb: FrameBuffer = undefined;
const TAB_WIDTH = 4;

var cursor: usize = 0;

var screen_width: usize = undefined;
var screen_height: usize = undefined;

var current_foreground: color = color.white();
var current_background: color = color.black();

pub fn init() void {
    fb.init();
    fb.clear(current_background);
    screen_width = fb.virtual_width / fonts.FONT_WIDTH;
    screen_height = fb.virtual_height / fonts.FONT_HEIGHT;
}

pub fn print(comptime format: []const u8, args: anytype) void {
    std.fmt.format(TerminalWriter{}, format, args) catch unreachable;
}

pub fn colorPrint(fg: color, comptime format: []const u8, args: anytype) void {
    const saved_fg = current_foreground;

    current_foreground = fg;
    print(format, args);
    current_foreground = saved_fg;
}

pub fn panic(comptime format: []const u8, args: anytype) void {
    colorPrint(color.red(), "\n\nKERNEL PANIC: " ++ format ++ "\n", args);
}

pub fn step(comptime format: []const u8, args: anytype) void {
    colorPrint(color.blue(), ">>> ", .{});
    print(format ++ "... ", args);
}

pub fn stepOk(comptime format: []const u8, args: anytype) void {
    if (format.len != 0) {
        print(format ++ " ", args);
    }
    colorPrint(color.green(), "[OK]\n", .{});
}

fn writeString(bytes: []const u8) void {
    for (bytes) |byte| {
        writeChar(byte);
    }
}

fn writeChar(c: u8) void {
    if (cursor == (screen_width * screen_height) - 1) {
        fb.scrollUp(current_background);
        cursor -= screen_width;
    }

    switch (c) {
        '\n' => while (true) {
            writeChar(' ');
            if (cursor % screen_width == 0) {
                break;
            }
        },

        '\t' => while (true) {
            writeChar(' ');
            if (cursor % TAB_WIDTH == 0) {
                break;
            }
        },

        else => {
            const x: usize = (cursor % screen_width) * fonts.FONT_WIDTH;
            const y: usize = (cursor / screen_width) * fonts.FONT_HEIGHT;
            fb.drawGlyph(c, @as(u32, @intCast(x)), @as(u32, @intCast(y)), current_foreground, current_background);
            cursor += 1;
        },
    }
}

var tw = TerminalWriter{};

const TerminalWriter = struct {
    const Self = @This();
    pub const Error = error{};
    const Writer = std.io.Writer(
        *TerminalWriter,
        error{},
        write,
    );

    pub fn write(_: *Self, bytes: []const u8) !usize {
        writeString(bytes);
        return bytes.len;
    }

    pub fn writeByte(_: Self, byte: u8) !void {
        writeString(&.{byte});
    }

    pub fn writeByteNTimes(_: Self, byte: u8, n: usize) !void {
        for (0..n) |_| {
            writeString(&.{byte});
        }
    }

    pub fn writeBytesNTimes(_: Self, bytes: []const u8, n: usize) !void {
        for (0..n) |_| {
            writeString(bytes);
        }
    }

    pub fn writeAll(_: Self, bytes: []const u8) !void {
        writeString(bytes);
    }

    pub fn writer(self: *TerminalWriter) Writer {
        return .{ .context = self };
    }
};

pub fn getWriter() *TerminalWriter {
    return &tw;
}
