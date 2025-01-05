namespace Memory {
    struct MMIO {
        unsigned long START;
        unsigned long RNG_START;
        unsigned long GPIO_START;
        unsigned long PL011_UART_START;
    };

    struct Map {
        static constexpr unsigned long BOARD_DEFAULT_LOAD_ADDRESS = 0x80000;

        static constexpr unsigned long RNG_OFFSET = 0x0010'4000;
        static constexpr unsigned long GPIO_OFFSET = 0x0020'0000;
        static constexpr unsigned long UART_OFFSET = 0x0020'1000;

        static MMIO getMMIO() {
            #if BOARD==bsp_rpi3
                return MMIO{
                    .START = 0x3F00'0000,
                    .RNG_START = 0x3F00'0000 + RNG_OFFSET,
                    .GPIO_START = 0x3F00'0000 + GPIO_OFFSET,
                    .PL011_UART_START = 0x3F00'0000 + UART_OFFSET,
                };
            #elif BOARD==bsp_rpi4
                return MMIO{
                    .START = 0xFE00'0000,
                    .RNG_START = 0xFE00'0000 + RNG_OFFSET,
                    .GPIO_START = 0xFE00'0000 + GPIO_OFFSET,
                    .PL011_UART_START = 0xFE00'0000 + UART_OFFSET,
                };
            #else
            #error Unknown board
            #endif
        }
    };
}