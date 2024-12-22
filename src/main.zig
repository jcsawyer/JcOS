const std = @import("std");
const mbox = @import("mbox.zig");
const uart = @import("uart.zig");
const rand = @import("rand.zig");
const delays = @import("delays.zig");
const power = @import("power.zig");

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

    var char: u8 = undefined;
    // Echo everything back
    while (true) {
        uart.uart_puts(" 1 - power off\n 2 - reset\nChoose one: ");
        char = uart.uart_getc();
        uart.uart_send(char);
        uart.uart_puts("\n\n");
        if (char == '1') {
            power.off();
        } else if (char == '2') {
            power.reset();
        }
    }
}
