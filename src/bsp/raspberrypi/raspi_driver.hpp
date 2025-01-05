#pragma once

#include "../../driver.hpp"
#include "../device_driver/bcm/bcm2xxx_gpio.hpp"
#include "../device_driver/bcm/bcm2xxx_pl011_uart.hpp"

namespace Driver::BSP::RaspberryPi {
    void init();
}