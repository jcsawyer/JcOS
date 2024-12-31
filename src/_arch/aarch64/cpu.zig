pub fn wait_forever() noreturn {
    while (true) {
        asm volatile ("wfe");
    }
}
