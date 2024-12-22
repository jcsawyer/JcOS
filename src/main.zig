const std = @import("std");
const mbox = @import("mbox.zig");
const uart = @import("uart.zig");

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

    // Say hello
    uart.uart_puts("Hello World!\n");

    mbox.mbox[0] = 8 * 4;
    mbox.mbox[1] = mbox.MBOX_REQUEST;
    mbox.mbox[2] = mbox.MBOX_TAG_GETSERIAL;
    mbox.mbox[3] = 8;
    mbox.mbox[4] = 8;
    mbox.mbox[5] = 0;
    mbox.mbox[6] = 0;

    mbox.mbox[7] = mbox.MBOX_TAG_LAST;

    if (mbox.call(mbox.MBOX_CH_PROP) == 0) {
        uart.uart_puts("Failed to get serial number!\n");
    } else {
        uart.uart_puts("Serial number: ");
        uart.uart_hex(mbox.mbox[6]);
        uart.uart_hex(mbox.mbox[5]);
        uart.uart_puts("\n");
    }

    // Echo everything back
    while (true) {
        const received = uart.uart_getc();
        uart.uart_send(received);
    }
}
