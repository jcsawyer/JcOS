pub fn spin_for_cycles(n: usize) void {
    for (0..n) |_| {
        asm volatile ("nop");
    }
}

pub fn wait_forever() noreturn {
    while (true) {
        asm volatile ("wfe");
    }
}

pub fn nop() void {
    asm volatile ("nop");
}
