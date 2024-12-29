const delays = @import("delays.zig");
const mbox = @import("mbox.zig");
const uart = @import("uart.zig");
const fonts = @import("fonts.zig");

pub const FrameBuffer = struct {
    depth: u32,
    physical_width: u32,
    physical_height: u32,
    pitch: u32,
    words: []volatile u32,
    size: u32,
    virtual_height: u32,
    virtual_width: u32,
    virtual_offset_x: u32,
    virtual_offset_y: u32,
    pixel_order: u32,

    pub fn clear(fb: *FrameBuffer, color: Color) void {
        @memset(fb.words, fb.colorTo32(color));
    }

    fn drawPixel(fb: *FrameBuffer, x: u32, y: u32, color: Color) void {
        fb.drawPixel32(x, y, color);
    }

    fn drawPixel32(fb: *FrameBuffer, x: u32, y: u32, color: Color) void {
        if (x >= fb.virtual_width or y >= fb.virtual_height) {
            uart.uart_puts("frame buffer does not fit in width and height\n");
        }
        fb.words[y * fb.pitch / 4 + x] = fb.colorTo32(color);
    }

    fn colorTo32(fb: *FrameBuffer, color: Color) u32 {
        if (fb.pixel_order == 0) {
            return (255 - @as(u32, @intCast(color.a)) << 24) | @as(u32, @intCast(color.b)) << 16 | @as(u32, @intCast(color.g)) << 8 | @as(u32, @intCast(color.r)) << 0;
        } else {
            return (255 - @as(u32, @intCast(color.a)) << 24) | @as(u32, @intCast(color.r)) << 16 | @as(u32, @intCast(color.g)) << 8 | @as(u32, @intCast(color.b)) << 0;
        }
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
            fb.pixel_order = mbox.mbox[24]; // get the actual channel order
            fb.virtual_width = 1024;
            fb.virtual_height = 768;
            fb.size = mbox.mbox[29]; // get the size of the framebuffer

            const raw_framebuffer: [*]volatile u32 = @ptrFromInt(@as(usize, mbox.mbox[28]));
            fb.words = raw_framebuffer[0 .. fb.physical_width * fb.physical_height];
        } else {
            uart.uart_puts("Unable to set screen resolution to 1024x768x32\n");
        }
    }

    pub fn scrollUp(fb: *FrameBuffer, color: Color) void {
        var i: u32 = 0;
        while (i < fb.virtual_height - 1) : (i += 1) {
            var j: u32 = 0;
            while (j < fb.virtual_width) : (j += 1) {
                fb.drawPixel(j, i, color);
            }
        }
        var k: u32 = 0;
        while (k < fb.virtual_width) : (k += 1) {
            fb.drawPixel(k, fb.virtual_height - 1, color);
        }
    }

    pub fn drawGlyph(fb: *FrameBuffer, ch: u8, x: u32, y: u32, fg: Color, bg: Color) void {
        const glyph = fonts.get_glyph(&fonts.font_list, ch);
        if (glyph) |g| {
            var gx: u32 = 0;
            while (gx < 8) : (gx += 1) {
                var gy: u32 = 0;
                while (gy < 16) : (gy += 1) {
                    if ((g.points[gy] >> @as(u3, @intCast(7 - gx))) & 0x01 != 0) {
                        fb.drawPixel(x + gx, y + gy, fg);
                    } else {
                        fb.drawPixel(x + gx, y + gy, bg);
                    }
                }
            }
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
    r: u8,
    g: u8,
    b: u8,
    a: u8,

    pub fn black() Color {
        return Color{ .r = 0, .g = 0, .b = 0, .a = 255 };
    }

    pub fn white() Color {
        return Color{ .r = 255, .g = 255, .b = 255, .a = 255 };
    }

    pub fn green() Color {
        return Color{ .r = 0, .g = 255, .b = 0, .a = 255 };
    }

    pub fn red() Color {
        return Color{ .r = 255, .g = 0, .b = 0, .a = 255 };
    }

    pub fn blue() Color {
        return Color{ .r = 0, .g = 0, .b = 255, .a = 255 };
    }

    pub fn yellow() Color {
        return Color{ .r = 255, .g = 255, .b = 0, .a = 255 };
    }
};
