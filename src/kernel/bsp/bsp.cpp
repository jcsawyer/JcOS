#include "bsp.hpp"

#if BOARD == bsp_rpi3 || BOARD == bsp_rpi4
#include "raspberrypi/raspberrypi.hpp"
#endif

namespace BSP {

void Board::init() {
#if BOARD == bsp_rpi3 || BOARD == bsp_rpi4
  Driver::BSP::RaspberryPi::RaspberryPi::init();
#else
#error Unknown board
#endif
}

} // namespace BSP