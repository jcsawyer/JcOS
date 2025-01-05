#include "driver.hpp"
#include "bcm2xxx_gpio.hpp"

namespace Driver {
    char* Driver::DeviceDriver::compatible() {
        return "brcm,bcm2xxx-gpio";
    }

    void Driver::DeviceDriver::init() {}
}