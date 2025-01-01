const std = @import("std");
const driver = @import("../../driver.zig");
const bcm_gpio = @import("../device_driver/bcm/bcm2xxx_gpio.zig").GPIO;
const bcm_pl011_uart = @import("../device_driver/bcm/bcm2xxx_pl011_uart.zig").PL011Uart;
const memory = @import("memory.zig").map;

var gpio: bcm_gpio = undefined;
var pl011_uart: bcm_pl011_uart = undefined;

fn postInitUart() anyerror!void {
    std.mem.doNotOptimizeAway(postInitUart);
    // console.register();
}

fn postInitGpio() anyerror!void {
    try gpio.map_pl011_uart();
}

fn driverUart() anyerror!void {
    pl011_uart = bcm_pl011_uart.new(memory.mmio.PL011_UART_START);
    const driver_impl = driver.DeviceDriver{
        .context = @ptrCast(&pl011_uart),
        .compatible = bcm_pl011_uart.compatible,
    };

    const driver_descriptor = driver.DeviceDriverDescriptor.new(driver_impl, postInitUart);
    driver.driver_manager().add_driver(driver_descriptor);
}

fn driverGpio() anyerror!void {
    gpio = bcm_gpio.new(memory.mmio.GPIO_START);
    const driver_impl = driver.DeviceDriver{
        .context = @ptrCast(&gpio),
        .compatible = bcm_gpio.compatible,
    };

    const driver_descriptor = driver.DeviceDriverDescriptor.new(driver_impl, postInitGpio);
    driver.driver_manager().add_driver(driver_descriptor);
}

pub fn init() anyerror!void {
    //try driverUart();
    try driverGpio();
}
