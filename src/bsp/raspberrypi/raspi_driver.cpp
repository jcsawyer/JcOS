#include "raspi_driver.hpp"
#include "memory.hpp"

#include "qemu_console.hpp"

namespace Driver::BSP::RaspberryPi {
    static Driver::BSP::BCM::GPIO gpio = Driver::BSP::BCM::GPIO(Memory::Map::getMMIO().GPIO_START);
    static Driver::BSP::BCM::UART uart = Driver::BSP::BCM::UART(Memory::Map::getMMIO().PL011_UART_START);
    static Driver::BSP::BCM::UART::UartConsole uartConsole = Driver::BSP::BCM::UART::UartConsole(&uart);
    //static Console::QemuConsole uartConsole = Console::QemuConsole();
    
    void postInitUart(){
        Console::setConsole(&uartConsole);
    }

    void posrtInitGpio(){
        gpio.mapPl011Uart();
    }

    void init() {
        Driver::driverManager().addDriver(Driver::DeviceDriverDescriptor(&gpio, &posrtInitGpio));
        Driver::driverManager().addDriver(Driver::DeviceDriverDescriptor(&uart, &postInitUart));
    }
}