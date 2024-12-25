const std = @import("std");
const gpio = @import("gpio.zig");
const mbox = @import("mbox.zig");

var UART0_DR: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00201000);
var UART0_FR: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00201018);
var UART0_IBRD: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00201024);
var UART0_FBRD: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00201028);
var UART0_LCRH: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x0020102C);
var UART0_CR: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00201030);
var UART0_IMSC: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00201038);
var UART0_ICR: *volatile u32 = @ptrFromInt(gpio.MMIO_BASE + 0x00201044);

fn delay(cycles: usize) void {
    var r = cycles;
    while (r != 0) : (r -= 1) {
        asm volatile ("nop");
    }
}

pub fn uart_init() void {
    var r: u32 = 0;

    // Initialize UART
    UART0_CR.* = 0; // Disable UART0

    // set up clock for consistent divisor values
    mbox.mbox[0] = 9 * 4;
    mbox.mbox[1] = mbox.MBOX_REQUEST;
    mbox.mbox[2] = mbox.MBOX_TAG_SETCLKRATE;
    mbox.mbox[3] = 12;
    mbox.mbox[4] = 8;
    mbox.mbox[5] = 2; // UART clock
    mbox.mbox[6] = 4000000; // 4Mhz
    mbox.mbox[7] = 0; // clear turbo
    mbox.mbox[8] = mbox.MBOX_TAG_LAST;
    _ = mbox.call(mbox.MBOX_CH_PROP);

    r = gpio.GPFSEL1.*;
    r &= @intCast(~(@as(u32, @intCast((7 << 12) | (7 << 15))))); // Clear GPIO 14, 15
    r |= (4 << 12) | (4 << 15); // Set alt0 for GPIO 14, 15
    gpio.GPFSEL1.* = r;
    gpio.GPPUD.* = 0; // Enable pins 14 and 15
    r = 150;
    while (r != 0) : (r -= 1) {
        asm volatile ("nop");
    }
    gpio.GPPUDCLK0.* = 0; // flush GPIO setup

    UART0_ICR.* = 0x7FF; // Clear pending interrupts
    UART0_IBRD.* = 2; // 115200 baud
    UART0_FBRD.* = 0xB;
    UART0_LCRH.* = 0x70; // 8n1, FIFO enabled
    UART0_CR.* = 0x301; // Enable UART0, Tx, Rx
}

pub fn uart_send(c: u32) void {
    // Wait until we can send
    while (UART0_FR.* & 0x20 != 0) {
        asm volatile ("nop");
    }
    // Write the character to the buffer
    UART0_DR.* = c;
}

pub fn uart_getc() u8 {
    // Wait until something is in the buffer
    while (UART0_FR.* & 0x10 != 0) {
        asm volatile ("nop");
    }
    // Read and return the character
    const r: u8 = @intCast(UART0_DR.*);
    if (r == '\r') {
        return '\n';
    } else {
        return r;
    }
}

pub fn uart_puts(s: []const u8) void {
    for (s) |c| {
        if (c == '\n') {
            uart_send('\r');
        }

        uart_send(c);
    }
}
pub fn uart_hex(d: u32) void {
    const charset = "0123456789ABCDEF";
    const bytes = u32_to_bytes(d);
    for (bytes) |byte| {
        uart_send(charset[byte >> 4]);
        uart_send(charset[byte & 0xF]);
    }
}
fn u32_to_bytes(n: u32) [4]u8 {
    return [4]u8{
        @intCast((n >> 24) & 0xFF), // Most significant byte
        @intCast((n >> 16) & 0xFF),
        @intCast((n >> 8) & 0xFF),
        @intCast(n & 0xFF), // Least significant byte
    };
}

pub fn uart_hex64(d: u64) void {
    const charset = "0123456789ABCDEF";
    const bytes = u64_to_bytes(d);
    for (bytes) |byte| {
        uart_send(charset[byte >> 4]);
        uart_send(charset[byte & 0xF]);
    }
}
fn u64_to_bytes(n: u64) [8]u8 {
    return [8]u8{
        @intCast((n >> 56) & 0xFF), // Most significant byte
        @intCast((n >> 48) & 0xFF),
        @intCast((n >> 40) & 0xFF),
        @intCast((n >> 32) & 0xFF),
        @intCast((n >> 24) & 0xFF),
        @intCast((n >> 16) & 0xFF),
        @intCast((n >> 8) & 0xFF),
        @intCast(n & 0xFF), // Least significant byte
    };
}
