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
        \\
        \\    // set up EL1
        \\    mrs     x0, CurrentEL
        \\    and     x0, x0, #12 // clear reserved bits
        \\
        \\    // running at EL3?
        \\    cmp     x0, #12
        \\    bne     5f
        \\    // should never be executed, just for completeness
        \\    mov     x2, #0x5b1
        \\    msr     scr_el3, x2
        \\    mov     x2, #0x3c9
        \\    msr     spsr_el3, x2
        \\    adr     x2, 5f
        \\    msr     elr_el3, x2
        \\    eret
        \\
        \\    // running at EL2?
        \\5:  cmp     x0, #4
        \\    beq     5f
        \\    msr     sp_el1, x1
        \\    // enable CNTP for EL1
        \\    mrs     x0, cnthctl_el2
        \\    orr     x0, x0, #3
        \\    msr     cnthctl_el2, x0
        \\    msr     cntvoff_el2, xzr
        \\    // enable AArch64 in EL1
        \\    mov     x0, #(1 << 31)      // AArch64
        \\    orr     x0, x0, #(1 << 1)   // SWIO hardwired on Pi3
        \\    msr     hcr_el2, x0
        \\    mrs     x0, hcr_el2
        \\    // Setup SCTLR access
        \\    mov     x2, #0x0800
        \\    movk    x2, #0x30d0, lsl #16
        \\    msr     sctlr_el1, x2
        \\    // set up exception handlers
        \\    ldr     x2, =_vectors
        \\    msr     vbar_el1, x2
        \\    // change execution level to EL1
        \\    mov     x2, #0x3c4
        \\    msr     spsr_el2, x2
        \\    adr     x2, 5f
        \\    msr     elr_el2, x2
        \\    eret
        \\
        \\5:  mov     sp, x1
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
        \\
        \\    // important, code has to be properly aligned
        \\    .align 11
        \\_vectors:
        \\    // synchronous
        \\    .align  7
        \\    mov     x0, #0
        \\    mrs     x1, esr_el1
        \\    mrs     x2, elr_el1
        \\    mrs     x3, spsr_el1
        \\    mrs     x4, far_el1
        \\    b       exc_handler
        \\
        \\    // IRQ
        \\    .align  7
        \\    mov     x0, #1
        \\    mrs     x1, esr_el1
        \\    mrs     x2, elr_el1
        \\    mrs     x3, spsr_el1
        \\    mrs     x4, far_el1
        \\    b       exc_handler
        \\
        \\    // FIQ
        \\    .align  7
        \\    mov     x0, #2
        \\    mrs     x1, esr_el1
        \\    mrs     x2, elr_el1
        \\    mrs     x3, spsr_el1
        \\    mrs     x4, far_el1
        \\    b       exc_handler
        \\
        \\    // SError
        \\    .align  7
        \\    mov     x0, #3
        \\    mrs     x1, esr_el1
        \\    mrs     x2, elr_el1
        \\    mrs     x3, spsr_el1
        \\    mrs     x4, far_el1
        \\    b       exc_handler
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

pub fn panic(message: []const u8, stack_trace: ?*std.builtin.StackTrace, ret_addr: ?usize) noreturn {
    _ = stack_trace;
    _ = ret_addr;

    uart.uart_puts("\n!KERNEL PANIC!\n");
    uart.uart_puts(message);
    uart.uart_puts("\n");

    fb.print(0, 0, "!KERNEL PANIC!\n");
    fb.print(0, 16, message);

    while (true) {}
}

export fn exc_handler(ex_type: u64, esr: u64, elr: u64, spsr: u64, far: u64) void {
    // Print out interruption type
    switch (ex_type) {
        0 => uart.uart_puts("Synchronous"),
        1 => uart.uart_puts("IRQ"),
        2 => uart.uart_puts("FIQ"),
        3 => uart.uart_puts("SError"),
        else => uart.uart_puts("Unknown"),
    }
    uart.uart_puts(": ");

    // Decode exception type
    switch (esr >> 26) {
        0b000000 => uart.uart_puts("Unknown"),
        0b000001 => uart.uart_puts("Trapped WFI/WFE"),
        0b001110 => uart.uart_puts("Illegal execution"),
        0b010101 => uart.uart_puts("System call"),
        0b100000 => uart.uart_puts("Instruction abort, lower EL"),
        0b100001 => uart.uart_puts("Instruction abort, same EL"),
        0b100010 => uart.uart_puts("Instruction alignment fault"),
        0b100100 => uart.uart_puts("Data abort, lower EL"),
        0b100101 => uart.uart_puts("Data abort, same EL"),
        0b100110 => uart.uart_puts("Stack alignment fault"),
        0b101100 => uart.uart_puts("Floating point"),
        else => uart.uart_puts("Unknown"),
    }

    // Decode data abort cause
    if (esr >> 26 == 0b100100 or esr >> 26 == 0b100101) {
        uart.uart_puts(", ");
        switch ((esr >> 2) & 0x3) {
            0 => uart.uart_puts("Address size fault"),
            1 => uart.uart_puts("Translation fault"),
            2 => uart.uart_puts("Access flag fault"),
            3 => uart.uart_puts("Permission fault"),
            else => {},
        }
        switch (esr & 0x3) {
            0 => uart.uart_puts(" at level 0"),
            1 => uart.uart_puts(" at level 1"),
            2 => uart.uart_puts(" at level 2"),
            3 => uart.uart_puts(" at level 3"),
            else => {},
        }
    }

    // Dump registers
    uart.uart_puts(":\n  ESR_EL1 ");
    uart.uart_hex64(esr >> 32);
    uart.uart_hex64(esr);
    uart.uart_puts(" ELR_EL1 ");
    uart.uart_hex64(elr >> 32);
    uart.uart_hex64(elr);
    uart.uart_puts("\n SPSR_EL1 ");
    uart.uart_hex64(spsr >> 32);
    uart.uart_hex64(spsr);
    uart.uart_puts(" FAR_EL1 ");
    uart.uart_hex64(far >> 32);
    uart.uart_hex64(far);
    uart.uart_puts("\n");

    // No return from exception for now
    while (true) {}
}

var fb: FrameBuffer = undefined;
