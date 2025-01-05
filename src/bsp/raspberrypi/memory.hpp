namespace Memory {
    struct MMIO {
        unsigned long START;
        unsigned long RNG_START;
        unsigned long GPIO_START;
        unsigned long PL011_UART_START;

        MMIO(unsigned long start, unsigned long rng_start, unsigned long gpio_start, unsigned long uart_start)
                : START(start), RNG_START(rng_start), GPIO_START(gpio_start), PL011_UART_START(uart_start) {}
    };

    struct Map {
        static const unsigned long BOARD_DEFAULT_LOAD_ADDRESS = 0x80000;

        static const unsigned long RNG_OFFSET = 0x00104000;
        static const unsigned long GPIO_OFFSET = 0x00200000;
        static const unsigned long UART_OFFSET = 0x00201000;

        static MMIO getMMIO() {
            #if BOARD == bsp_rpi3
                return MMIO(
                    0x3F000000,
                    0x3F000000 + RNG_OFFSET,
                    0x3F000000 + GPIO_OFFSET,
                    0x3F000000 + UART_OFFSET
                );
            #elif BOARD == bsp_rpi4
                return MMIO(
                    0xFE000000,
                    0xFE000000 + RNG_OFFSET,
                    0xFE000000 + GPIO_OFFSET,
                    0xFE000000 + UART_OFFSET
                );
            #else
            #error Unknown board
            #endif
        }
    };
}
