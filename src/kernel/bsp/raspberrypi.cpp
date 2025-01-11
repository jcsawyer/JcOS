#include <bsp/bsp.hpp>

// TODO read from board
#define bsp_rpi3 1
#define bsp_rpi4 2
#define bsp_rpi3b 3
#define bsp_rpi3bplus 4

const char *BSP::DeviceName() {
#if BOARD == bsp_rpi3
  return "Raspberry Pi 3";
#elif BOARD == bsp_rpi3b
  return "Raspberry Pi 3B";
#elif BOARD == bsp_rpi3bplus
  return "Raspberry Pi 3+";
#elif BOARD == bsp_rpi4
  return "Raspberry Pi 4";
#else
#error Unknown board
#endif
}
