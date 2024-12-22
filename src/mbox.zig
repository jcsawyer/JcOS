const std = @import("std");
const gpio = @import("gpio.zig");
const AtomicOrder = std.builtin.AtomicOrder;

pub const MBOX_REQUEST = 0;

pub const MBOX_CH_POWER = 0;
pub const MBOX_CH_FB = 1;
pub const MBOX_CH_VUART = 2;
pub const MBOX_CH_VCHIQ = 3;
pub const MBOX_CH_LEDS = 4;
pub const MBOX_CH_BTNS = 5;
pub const MBOX_CH_TOUCH = 6;
pub const MBOX_CH_COUNT = 7;
pub const MBOX_CH_PROP = 8;

pub const MBOX_TAG_GETSERIAL = 0x10004;
pub const MBOX_TAG_LAST = 0;

const VIDEOCORE_MBOX = gpio.MMIO_BASE + 0xB880;
var MBOX_READ: *volatile u32 = @ptrFromInt(VIDEOCORE_MBOX + 0x0);
var MBOX_POLL: *volatile u32 = @ptrFromInt(VIDEOCORE_MBOX + 0x10);
var MBOX_SENDER: *volatile u32 = @ptrFromInt(VIDEOCORE_MBOX + 0x14);
var MBOX_STATUS: *volatile u32 = @ptrFromInt(VIDEOCORE_MBOX + 0x18);
var MBOX_CONFIG: *volatile u32 = @ptrFromInt(VIDEOCORE_MBOX + 0x1C);
var MBOX_WRITE: *volatile u32 = @ptrFromInt(VIDEOCORE_MBOX + 0x20);
const MBOX_RESPONSE = 0x80000000;
const MBOX_FULL = 0x80000000;
const MBOX_EMPTY = 0x40000000;

pub var mbox: [36]u32 align(16) = undefined;

pub fn call(ch: u8) i32 {
    const mask: u32 = 0x0F;
    const ptr = @intFromPtr(&mbox);
    const r: usize = (ptr & ~mask) | (ch & 0xF);

    // Wait until we can write to the mailbox
    while (@intFromPtr(MBOX_STATUS) & MBOX_FULL != 0) {
        asm volatile ("nop");
    }

    // Write the address of our message to the mailbox with channel identifier
    MBOX_WRITE.* = @intCast(r);

    // Wait for the response
    while (true) {
        // Is there a response?
        while (MBOX_STATUS.* & MBOX_EMPTY != 0) {
            asm volatile ("nop");
        }

        // Is it a response to our message?
        if (r == MBOX_READ.*) {
            // Is it a valid successful response?
            if (mbox[1] == MBOX_RESPONSE) {
                return 1;
            } else {
                return 0;
            }
        }
    }
}
