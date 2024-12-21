const std = @import("std");

const MMIO_BASE: u32 = 0x3F000000; // Update this based on your Raspberry Pi model
const AUX_ENABLE = MMIO_BASE + 0x00215004;
const AUX_MU_IO = MMIO_BASE + 0x00215040;
const AUX_MU_IER = MMIO_BASE + 0x00215044;
const AUX_MU_IIR = MMIO_BASE + 0x00215048;
const AUX_MU_LCR = MMIO_BASE + 0x0021504C;
const AUX_MU_MCR = MMIO_BASE + 0x00215050;
const AUX_MU_LSR = MMIO_BASE + 0x00215054;
const AUX_MU_CNTL = MMIO_BASE + 0x00215060;
const AUX_MU_BAUD = MMIO_BASE + 0x00215068;
const GPFSEL1 = MMIO_BASE + 0x00200004;
const GPPUD = MMIO_BASE + 0x00200094;
const GPPUDCLK0 = MMIO_BASE + 0x00200098;

pub fn mmio_write(reg: usize, data: u32) void {
    const ptr: *volatile u32 = @ptrFromInt(reg);
    ptr.* = data;
}

pub fn mmio_read(reg: usize) u32 {
    const ptr: *volatile u32 = @ptrFromInt(reg);
    return ptr.*;
}

fn delay(cycles: usize) void {
    var r = cycles;
    while (r != 0) : (r -= 1) {
        asm volatile ("nop");
    }
}

pub fn uart_init() void {
    var r: u32 = 0;

    // Initialize UART
    mmio_write(AUX_ENABLE, mmio_read(AUX_ENABLE) | 1); // Enable UART1, AUX mini UART
    mmio_write(AUX_MU_CNTL, 0);
    mmio_write(AUX_MU_LCR, 3); // 8 bits
    mmio_write(AUX_MU_MCR, 0);
    mmio_write(AUX_MU_IER, 0);
    mmio_write(AUX_MU_IIR, 0xC6); // Disable interrupts
    mmio_write(AUX_MU_BAUD, 270); // Set baud rate to 115200

    // Map UART1 to GPIO pins
    r = mmio_read(GPFSEL1);
    r &= @intCast(~(@as(u32, @intCast((7 << 12) | (7 << 15))))); // Clear GPIO 14, 15
    r &= @intCast(~(@as(u32, @intCast((7 << 12) | (7 << 15))))); // Clear GPIO 14, 15
    r |= (2 << 12) | (2 << 15); // Set alt5 for GPIO 14, 15
    mmio_write(GPFSEL1, r);

    mmio_write(GPPUD, 0); // Disable pull-up/down
    delay(150);

    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15)); // Enable pins 14, 15
    delay(150);

    mmio_write(GPPUDCLK0, 0); // Flush GPIO setuppub

    mmio_write(AUX_MU_CNTL, 3); // Enable Tx, Rx
}

pub fn uart_send(c: u32) void {
    // Wait until we can send
    while ((mmio_read(AUX_MU_LSR) & 0x20) == 0) {
        asm volatile ("nop");
    }
    // Write the character to the buffer
    mmio_write(AUX_MU_IO, c);
}

pub fn uart_getc() u8 {
    // Wait until something is in the buffer
    while ((mmio_read(AUX_MU_LSR) & 0x01) == 0) {
        asm volatile ("nop");
    }
    // Read and return the character
    const r: u8 = @intCast(mmio_read(AUX_MU_IO));
    return if (r == '\r') '\n' else r;
}

pub fn uart_puts(s: []const u8) void {
    var i: usize = 0;
    while (i < s.len) : (i += 1) {
        const c = s[i];
        if (c == '\n') {
            uart_send('\r');
        }
        uart_send(c);
    }
}
