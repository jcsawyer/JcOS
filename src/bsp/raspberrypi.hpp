namespace RaspberryPi {
    static char* boardName() {
        #if BOARD==bsp_rpi3
            return "Raspberry Pi 3";
        #elif BOARD==bsp_rpi4
            return "Raspberry Pi 4";
        #else
        #error Unknown board
        #endif
    }
}