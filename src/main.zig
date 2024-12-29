const std = @import("std");
const mbox = @import("mbox.zig");
const uart = @import("uart.zig");
const rand = @import("rand.zig");
const delays = @import("delays.zig");
const power = @import("power.zig");
const FrameBuffer = @import("framebuffer.zig").FrameBuffer;
const Color = @import("framebuffer.zig").Color;
const terminal = @import("terminal.zig");
const time = @import("time.zig");

export fn main() void {
    time.init();
    terminal.init();

    terminal.step("Initializing terminal", .{});
    terminal.stepOk("", .{});

    terminal.step("Initializing UART", .{});
    uart.uart_init();
    terminal.stepOk("", .{});

    const current_el: usize = asm ("mrs %[current_el], CurrentEL"
        : [current_el] "=r" (-> usize),
    );
    uart.uart_puts("Current exception level: ");
    uart.uart_hex(@as(u32, @intCast(current_el)));
    uart.uart_puts("\n");

    terminal.step("Initializing random ", .{});
    rand.init();

    terminal.stepOk("", .{});
    terminal.print("\n", .{});

    terminal.print("ARM Exception Level: 0x{X:0>8}\n", .{current_el});
    terminal.print("\n", .{});
    terminal.print("\n", .{});

    // Echo everything back
    while (true) {
        time.update();
        terminal.print("{c}", .{uart.uart_getc()});
    }
}

pub fn panic(message: []const u8, stack_trace: ?*std.builtin.StackTrace, ret_addr: ?usize) noreturn {
    _ = stack_trace;
    _ = ret_addr;

    uart.uart_puts("\n!KERNEL PANIC!\n");
    uart.uart_puts(message);
    uart.uart_puts("\n");

    terminal.panic("{s}\n > Press any key to restart...", .{message});

    while (true) {
        _ = uart.uart_getc();
        power.reset();
    }
}

export fn exc_handler(ex_type: u64, esr: u64, elr: u64, spsr: u64, far: u64) void {

    // Print out interruption type
    switch (ex_type) {
        0 => uart.uart_puts("Synchronous"),
        1 => uart.uart_puts("IRQ"),
        2 => uart.uart_puts("FIQ"),
        3 => uart.uart_puts("SError"),
        else => uart.uart_puts("Unknown"),
    }
    uart.uart_puts(": ");

    // Decode exception type
    switch (esr >> 26) {
        0b000000 => uart.uart_puts("Unknown"),
        0b000001 => uart.uart_puts("Trapped WFI/WFE"),
        0b001110 => uart.uart_puts("Illegal execution"),
        0b010101 => uart.uart_puts("System call"),
        0b100000 => uart.uart_puts("Instruction abort, lower EL"),
        0b100001 => uart.uart_puts("Instruction abort, same EL"),
        0b100010 => uart.uart_puts("Instruction alignment fault"),
        0b100100 => uart.uart_puts("Data abort, lower EL"),
        0b100101 => uart.uart_puts("Data abort, same EL"),
        0b100110 => uart.uart_puts("Stack alignment fault"),
        0b101100 => uart.uart_puts("Floating point"),
        else => uart.uart_puts("Unknown"),
    }

    // Decode data abort cause
    if (esr >> 26 == 0b100100 or esr >> 26 == 0b100101) {
        uart.uart_puts(", ");
        switch ((esr >> 2) & 0x3) {
            0 => uart.uart_puts("Address size fault"),
            1 => uart.uart_puts("Translation fault"),
            2 => uart.uart_puts("Access flag fault"),
            3 => uart.uart_puts("Permission fault"),
            else => {},
        }
        switch (esr & 0x3) {
            0 => uart.uart_puts(" at level 0"),
            1 => uart.uart_puts(" at level 1"),
            2 => uart.uart_puts(" at level 2"),
            3 => uart.uart_puts(" at level 3"),
            else => {},
        }
    }

    // Dump registers
    uart.uart_puts(":\n  ESR_EL1 ");
    uart.uart_hex(@as(u32, @intCast(esr >> 32)));
    uart.uart_hex(@as(u32, @intCast(esr)));
    uart.uart_puts(" ELR_EL1 ");
    uart.uart_hex(@as(u32, @intCast(elr >> 32)));
    uart.uart_hex(@as(u32, @intCast(elr)));
    uart.uart_puts("\n SPSR_EL1 ");
    uart.uart_hex(@as(u32, @intCast(spsr >> 32)));
    uart.uart_hex(@as(u32, @intCast(spsr)));
    uart.uart_puts(" FAR_EL1 ");
    uart.uart_hex(@as(u32, @intCast(far >> 32)));
    uart.uart_hex(@as(u32, @intCast(far)));
    uart.uart_puts("\n");

    // No return from exception for now
    while (true) {}
}

var fb: FrameBuffer = undefined;

pub const ArgSetType = u32;
const max_format_args = @typeInfo(ArgSetType).Int.bits;

fn aarch64_test(comptime args: anytype, writer: anytype) !void {
    const ArgsType = @TypeOf(args);
    const args_type_info = @typeInfo(ArgsType);
    if (args_type_info != .Struct) {
        @compileError("expected tuple or struct argument, found " ++ @typeName(ArgsType));
    }

    const fields_info = args_type_info.Struct.fields;
    if (fields_info.len > max_format_args) {
        @compileError("32 arguments max are supported per format call");
    }

    comptime var arg_state: std.fmt.ArgState = .{ .args_len = fields_info.len };

    comptime var i = 0;
    inline while (i < fields_info.len) : (i += 1) {
        const arg_to_print = comptime arg_state.nextArg(i) orelse
            @compileError("too few arguments");
        try aarch64_test_sub(
            @field(args, fields_info[arg_to_print].name),
            writer,
            5,
        );
    }
}

const ANY = "any";
fn defaultSpec(comptime T: type) [:0]const u8 {
    switch (@typeInfo(T)) {
        .Array => |_| return ANY,
        .Pointer => |ptr_info| switch (ptr_info.size) {
            .One => switch (@typeInfo(ptr_info.child)) {
                .Array => |_| return ANY,
                else => {},
            },
            .Many, .C => return "*",
            .Slice => return ANY,
        },
        .Optional => |info| return "?" ++ defaultSpec(info.child),
        .ErrorUnion => |info| return "!" ++ defaultSpec(info.payload),
        else => {},
    }
    return "";
}
fn aarch64_test_sub(value: anytype, writer: anytype, max_depth: usize) @TypeOf(writer).Error!void {
    const T = @TypeOf(value);
    const actual_fmt: []const u8 = "s";

    uart.uart_puts(value);

    uart.uart_puts(@typeName(T));

    switch (@typeInfo(T)) {
        .Pointer => |ptr_info| {
            switch (ptr_info.size) {
                .One => {
                    if (ptr_info.child == u8) {
                        uart.uart_puts("u8");
                        return std.fmt.formatBuf(value[0..], std.fmt.FormatOptions{}, writer);
                    }
                    try writer.writeAll("{ ");
                    for (value, 0..) |elem, i| {
                        try std.fmt.formatType(elem, actual_fmt, std.fmt.FormatOptions{}, writer, max_depth - 1);
                        if (i != value.len - 1) {
                            try writer.writeAll(", ");
                        }
                    }
                    try writer.writeAll(" }");
                },
                .Many, .C => {},
                .Slice => {},
            }
        },
        else => @compileError("unable to format type '" ++ @typeName(T) ++ "'"),
    }
}
