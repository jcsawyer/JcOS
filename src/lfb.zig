const std = @import("std");
const uart = @import("uart.zig");
const mbox = @import("mbox.zig");
const delays = @import("delays.zig");
const homer = @cImport({
    @cInclude("homer.h");
});

var width: u32 = 0;
var height: u32 = 0;
var pitch: u32 = 0;
var isrgb: u32 = 0; // dimensions and channel order
var lfb: ?[*]u8 = null; // raw frame buffer address

/// Set screen resolution to 1024x768
pub fn lfb_init() void {
    // newer qemu segfaults if we don't wait here a bit
    delays.wait_msec(100000);

    mbox.mbox[0] = 35 * 4;
    mbox.mbox[1] = mbox.MBOX_REQUEST;

    mbox.mbox[2] = 0x48003; // set phy wh
    mbox.mbox[3] = 8;
    mbox.mbox[4] = 8;
    mbox.mbox[5] = 1024; // FrameBufferInfo.width
    mbox.mbox[6] = 768; // FrameBufferInfo.height

    mbox.mbox[7] = 0x48004; // set virt wh
    mbox.mbox[8] = 8;
    mbox.mbox[9] = 8;
    mbox.mbox[10] = 1024; // FrameBufferInfo.virtual_width
    mbox.mbox[11] = 768; // FrameBufferInfo.virtual_height

    mbox.mbox[12] = 0x48009; // set virt offset
    mbox.mbox[13] = 8;
    mbox.mbox[14] = 8;
    mbox.mbox[15] = 0; // FrameBufferInfo.x_offset
    mbox.mbox[16] = 0; // FrameBufferInfo.y.offset

    mbox.mbox[17] = 0x48005; // set depth
    mbox.mbox[18] = 4;
    mbox.mbox[19] = 4;
    mbox.mbox[20] = 32; // FrameBufferInfo.depth

    mbox.mbox[21] = 0x48006; // set pixel order
    mbox.mbox[22] = 4;
    mbox.mbox[23] = 4;
    mbox.mbox[24] = 1; // RGB, not BGR preferably

    mbox.mbox[25] = 0x40001; // get framebuffer
    mbox.mbox[26] = 8;
    mbox.mbox[27] = 8;
    mbox.mbox[28] = 4096; // FrameBufferInfo.pointer
    mbox.mbox[29] = 0; // FrameBufferInfo.size

    mbox.mbox[30] = 0x40008; // get pitch
    mbox.mbox[31] = 4;
    mbox.mbox[32] = 4;
    mbox.mbox[33] = 0; // FrameBufferInfo.pitch

    mbox.mbox[34] = mbox.MBOX_TAG_LAST;

    // Send the mailbox request
    if (mbox.call(mbox.MBOX_CH_PROP) != 0 and mbox.mbox[20] == 32 and mbox.mbox[28] != 0) {
        mbox.mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
        width = mbox.mbox[5]; // get actual physical width
        height = mbox.mbox[6]; // get actual physical height
        pitch = mbox.mbox[33]; // get number of bytes per line
        isrgb = mbox.mbox[24]; // get the actual channel order
        lfb = @ptrFromInt(@as(usize, mbox.mbox[28]));
    } else {
        uart.uart_puts("Unable to set screen resolution to 1024x768x32\n");
    }
}

pub fn lfb_showpicture() void {
    const lfb_ptr = lfb orelse return; // Safely unwrap the optional, or return if `lfb` is null
    var ptr: [*]u8 = lfb_ptr;
    var i: u32 = 0;
    var color: u32 = 0;

    const data: [*]const u8 = homer.homer_data;

    ptr += ((height - homer.homer_height) / 2) * pitch + (width - homer.homer_width) * 2;
    for (0..homer.homer_height) |_| {
        for (0..homer.homer_width) |_| {
            var pixel: [4]u8 = undefined;
            pixel[3] = (data[i + 0] - 33) << 2 | (data[i + 1] - 33) >> 4;
            pixel[2] = ((data[i + 1] - 33) & 0xF) << 4 | (data[i + 2] - 33) >> 2;
            pixel[1] = ((data[i + 2] - 33) & 0x3) << 6 | (data[i + 3] - 33);

            if (isrgb != 0) {
                color = @as(u32, pixel[0]) << 24 | @as(u32, pixel[1]) << 16 |
                    @as(u32, pixel[2]) << 8 | @as(u32, pixel[3]);
            } else {
                color = (@as(u32, pixel[0]) << 16) | (@as(u32, pixel[1]) << 8) | @as(u32, pixel[2]);
            }

            @as(*u32, @alignCast(@ptrCast(ptr))).* = color;

            ptr += 4;
            i += 4;
        }
        ptr += pitch - homer.homer_width * 4;
    }
}
