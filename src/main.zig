const std = @import("std");
const mbox = @import("mbox.zig");
const uart = @import("uart.zig");
const rand = @import("rand.zig");
const delays = @import("delays.zig");
const power = @import("power.zig");
const FrameBuffer = @import("framebuffer.zig").FrameBuffer;
const Color = @import("framebuffer.zig").Color;

comptime {
    asm (
        \\.section ".text.boot"
        \\
        \\.global _start
        \\
        \\_start:
        \\    // read cpu id, stop slave cores
        \\    mrs     x1, mpidr_el1
        \\    and     x1, x1, #3
        \\    cbz     x1, 2f
        \\    // cpu id > 0, stop
        \\1:  wfe
        \\    b       1b
        \\2:  // cpu id == 0
        \\
        \\    // set top of stack just before our code (stack grows to a lower address per AAPCS64)
        \\    ldr     x1, =_start
        \\    mov     sp, x1
        \\
        \\    // clear bss
        \\    ldr     x1, =__bss_start
        \\    ldr     w2, =__bss_size
        \\3:  cbz     w2, 4f
        \\    str     xzr, [x1], #8
        \\    sub     w2, w2, #1
        \\    cbnz    w2, 3b
        \\
        \\    // jump to C code, should not return
        \\4:  bl      main
        \\    // for failsafe, halt this core too
        \\    b       1b
    );
}

export fn main() void {
    // Set up the serial console
    uart.uart_init();
    rand.init();
    fb.init();
    //fb.clear(Color{ .red = 255, .green = 255, .blue = 255, .alpha = 255 });

    fb.print(10, 10, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_\\ ~!@#$%^&*()_+`1234567890-=[]{}|;':,.<>?{}/");

    // Echo everything back
    while (true) {
        uart.uart_send(uart.uart_getc());
    }
}

var fb: FrameBuffer = undefined;
