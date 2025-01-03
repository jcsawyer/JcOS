const std = @import("std");
const config = @import("config");
const cpu = @import("../../../cpu.zig").cpu;
const DeviceDriver = @import("../../../driver.zig").DeviceDriver;
const bsp_console = @import("../../../console.zig").Console;
const driver = @import("../../raspberrypi/driver.zig");
const fmt = @import("../../../std/fmt.zig");

const RegisterBlock = struct {
    DR: *volatile u32,
    FR: *volatile u32,
    IBRD: *volatile u32,
    FBRD: *volatile u32,
    LCR_H: *volatile u32,
    CR: *volatile u32,
    IMSC: *volatile u32,
    ICR: *volatile u32,

    fn new(mmio_start_addr: usize) RegisterBlock {
        return .{
            .DR = @ptrFromInt(mmio_start_addr + 0x00),
            .FR = @ptrFromInt(mmio_start_addr + 0x18),
            .IBRD = @ptrFromInt(mmio_start_addr + 0x24),
            .FBRD = @ptrFromInt(mmio_start_addr + 0x28),
            .LCR_H = @ptrFromInt(mmio_start_addr + 0x2C),
            .CR = @ptrFromInt(mmio_start_addr + 0x30),
            .IMSC = @ptrFromInt(mmio_start_addr + 0x38),
            .ICR = @ptrFromInt(mmio_start_addr + 0x44),
        };
    }
};

const BlockingMode = enum {
    Blocking,
    NonBlocking,
};

const PL011UartInner = struct {
    registers: RegisterBlock,
    chars_written: usize,
    chars_read: usize,

    pub fn new(mmio_start_addr: usize) PL011UartInner {
        return .{
            .registers = RegisterBlock.new(mmio_start_addr),
            .chars_written = 0,
            .chars_read = 0,
        };
    }

    /// Set up baud rate and characteristics.
    ///
    /// This results in 8N1 and 921_600 baud.
    ///
    /// The calculation for the BRD is (we set the clock to 48 MHz in config.txt):
    /// `(48_000_000 / 16) / 921_600 = 3.2552083`.
    ///
    /// This means the integer part is `3` and goes into the `IBRD`.
    /// The fractional part is `0.2552083`.
    ///
    /// `FBRD` calculation according to the PL011 Technical Reference Manual:
    /// `INTEGER((0.2552083 * 64) + 0.5) = 16`.
    ///
    /// Therefore, the generated baud rate divider is: `3 + 16/64 = 3.25`. Which results in a
    /// genrated baud rate of `48_000_000 / (16 * 3.25) = 923_077`.
    ///
    /// Error = `((923_077 - 921_600) / 921_600) * 100 = 0.16%`.
    pub fn init(self: *PL011UartInner) void {
        // Execution can arrive here while there are still characters queued in the TX FIFO and
        // actively being sent out by the UART hardware. If the UART is turned off in this case,
        // those queued characters would be lost.
        //
        // For example, this can happen during runtime on a call to panic!(), because panic!()
        // initializes its own UART instance and calls init().
        //
        // Hence, flush first to ensure all pending characters are transmitted.
        self.flush();

        // Turn the UART off temporarily
        //const cr: *volatile u32 = @ptrFromInt(0x3F00_0000 + 0x0020_1000);
        //cr.* = 0;
        self.registers.CR.* = 0;

        // Clear all pending interrupts
        self.registers.ICR.* = 0x7FF;

        // From the PL011 Technical Reference Manual:
        //
        // The LCR_H, IBRD, and FBRD registers form the single 30-bit wide LCR Register that is
        // updated on a single write strobe generated by a LCR_H write. So, to internally update the
        // contents of IBRD or FBRD, a LCR_H write must always be performed at the end.
        //
        // Set the baud rate, 8N1 and FIFO enabled.
        self.registers.IBRD.* = 3;
        self.registers.FBRD.* = 16;
        self.registers.LCR_H.* = 0b11 << 5 | 1 << 4;

        // Enable UART and both RX and TX
        self.registers.CR.* = 0b1 | 0b10 | 0b100;
    }

    fn write_char(self: *PL011UartInner, c: u8) void {
        // Wait until there is space in the FIFO
        while (self.registers.FR.* & 0x20 != 0) {
            cpu.nop();
        }

        // Write the character to the FIFO
        self.registers.DR.* = c;
        self.chars_written += 1;
    }

    fn read_char(self: *PL011UartInner, blocking_mode: BlockingMode) u8 {
        // If RX FIFI is empty
        if (self.registers.FR.* & 0x10 != 0) {
            // Immediately return in non-blocking mode
            if (blocking_mode == BlockingMode.NonBlocking) {
                return 0;
            }

            // Otherwise, wait until a character is received
            while (self.registers.FR.* & 0x10 != 0) {
                cpu.nop();
            }
        }

        // Read the character from the FIFO
        const ret: u8 = @intCast(self.registers.DR.*);
        self.chars_read += 1;
        return ret;
    }

    fn flush(self: *PL011UartInner) void {
        while (self.registers.FR.* & 0x20 != 0) {
            cpu.nop();
        }
    }
};

pub const PL011Uart = struct {
    const Self = @This();
    inner: PL011UartInner,

    pub fn new(mmio_start_addr: usize) PL011Uart {
        return .{
            .inner = PL011UartInner.new(mmio_start_addr),
        };
    }

    pub fn compatible(_: *anyopaque) []const u8 {
        return "PL011 UART";
    }

    pub fn printChar(self: *PL011Uart, char: u8) void {
        self.inner.write_char(char);
    }

    pub fn init(ctx: *anyopaque) anyerror!void {
        const self: *PL011Uart = @alignCast(@ptrCast(ctx));
        return self.inner.init();
    }

    const vtable = DeviceDriver.VTable{
        .compatible = PL011Uart.compatible,
        .init = PL011Uart.init,
    };

    pub fn driver(pl011_uart: *PL011Uart) DeviceDriver {
        return DeviceDriver{
            .ctx = @ptrCast(pl011_uart),
            .vtable = &PL011Uart.vtable,
        };
    }
};

var console = UARTConsole{};

pub fn getConsole() bsp_console {
    return console.console();
}

pub const UARTConsole = struct {
    const Self = @This();

    fn flush(_: *anyopaque) void {
        driver.pl011_uart.inner.flush();
    }

    fn clearRx(_: *anyopaque) void {
        // Read from the RX FIFO until it is empty
        while (driver.pl011_uart.inner.read_char(BlockingMode.NonBlocking) != 0) {}
    }

    fn print(_: *anyopaque, comptime str: []const u8, args: anytype) void {
        fmt.format(ConsoleWriter{ .context = {} }, str, args) catch unreachable;
    }

    fn printChar(_: *anyopaque, char: u8) void {
        driver.pl011_uart.inner.write_char(char);
    }

    fn readChar(_: *anyopaque) u8 {
        return driver.pl011_uart.inner.read_char(BlockingMode.Blocking);
    }

    const vtable = bsp_console.VTable{
        .flush = UARTConsole.flush,
        .clearRx = UARTConsole.clearRx,
        .print = UARTConsole.print,
        .printChar = UARTConsole.printChar,
        .readChar = UARTConsole.readChar,
    };

    pub fn console(self: *UARTConsole) bsp_console {
        return .{
            .context = self,
            .vtable = &UARTConsole.vtable,
        };
    }
};

const Context = void;
const WriteError = anyerror;
fn writeFn(_: Context, buf: []const u8) anyerror!usize {
    for (buf) |char| {
        driver.pl011_uart.inner.write_char(char);
    }
    return buf.len;
}
const ConsoleWriter = std.io.GenericWriter(Context, WriteError, writeFn);
