const delays = @import("delays.zig");
const mbox = @import("mbox.zig");
const uart = @import("uart.zig");

pub const FrameBuffer = struct {
    depth: u32,
    physical_width: u32,
    physical_height: u32,
    pitch: u32,
    words: [*]volatile u32,
    virtual_height: u32,
    virtual_width: u32,
    virtual_offset_x: u32,
    virtual_offset_y: u32,

    pub fn clear(fb: *FrameBuffer, color: Color) void {
        var y: u32 = 0;
        while (y < fb.virtual_height) : (y += 1) {
            var x: u32 = 0;
            while (x < fb.virtual_width) : (x += 1) {
                fb.drawPixel(x, y, color);
            }
        }
    }

    fn drawPixel(fb: *FrameBuffer, x: u32, y: u32, color: Color) void {
        fb.drawPixel32(x, y, color.to32());
    }

    fn drawPixel32(fb: *FrameBuffer, x: u32, y: u32, color: u32) void {
        if (x >= fb.virtual_width or y >= fb.virtual_height) {
            uart.uart_puts("frame buffer does no tfit in width and height\n");
        }
        fb.words[y * fb.pitch / 4 + x] = color;
    }

    pub fn init(fb: *FrameBuffer) void {
        delays.wait_msec(100_000);

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
            fb.physical_width = mbox.mbox[5]; // get actual physical width
            fb.physical_height = mbox.mbox[6]; // get actual physical height
            fb.pitch = mbox.mbox[33]; // get number of bytes per line
            //isrgb = mbox.mbox[24]; // get the actual channel order
            fb.words = @ptrFromInt(@as(usize, mbox.mbox[28]));

            fb.virtual_width = 1024;
            fb.virtual_height = 768;
        } else {
            uart.uart_puts("Unable to set screen resolution to 1024x768x32\n");
        }
    }
};

fn getU32(base: [*]const u8, offset: u32) u32 {
    var word: u32 = 0;
    var i: u32 = 0;
    while (i <= 3) : (i += 1) {
        word >>= 8;
        word |= @as(u32, @intCast(@as(*u8, @ptrFromInt(@intFromPtr(base) + offset + i)).*)) << 24;
    }
    return word;
}
pub const Color = struct {
    red: u8,
    green: u8,
    blue: u8,
    alpha: u8,

    fn to32(color: Color) u32 {
        return (255 - @as(u32, @intCast(color.alpha)) << 24) | @as(u32, @intCast(color.red)) << 16 | @as(u32, @intCast(color.green)) << 8 | @as(u32, @intCast(color.blue)) << 0;
    }
};
