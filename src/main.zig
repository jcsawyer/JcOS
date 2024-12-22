const std = @import("std");
const mbox = @import("mbox.zig");
const uart = @import("uart.zig");
const rand = @import("rand.zig");
const delays = @import("delays.zig");

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

    uart.uart_puts("Waiting 1_000_000 CPU cycles (ARM CPU): ");
    delays.wait_cycles(1_000_000);
    uart.uart_puts("OK\n");

    uart.uart_puts("Waiting 1_000_000 microseconds (ARM CPU): ");
    delays.wait_msec(1_000_000);
    uart.uart_puts("OK\n");

    uart.uart_puts("Waiting 1_000_000 microseconds (System Timer): ");
    if (delays.get_system_timer() == 0) {
        uart.uart_puts("System Timer not available\n");
    } else {
        delays.wait_msec_st(1_000_000);
        uart.uart_puts("OK\n");
    }

    uart.uart_puts("Here goes a random number: ");
    uart.uart_hex(rand.next(0, 4294967294));
    uart.uart_puts("\n");

    // Echo everything back
    while (true) {
        const received = uart.uart_getc();
        uart.uart_send(received);
    }
}
