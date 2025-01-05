const std = @import("std");
const driver = @import("../../driver.zig");
const bcm_gpio = @import("../device_driver/bcm/bcm2xxx_gpio.zig").GPIO;
const bcm_pl011_uart = @import("../device_driver/bcm/bcm2xxx_pl011_uart.zig").PL011Uart;
const bcm_rng = @import("../device_driver/bcm/bcm2xxx_rng.zig").RNG;
const memory = @import("memory.zig").map;

var gpio: bcm_gpio = undefined;
pub var pl011_uart: bcm_pl011_uart = undefined;
pub var rng: bcm_rng = undefined;

fn postInitUart() anyerror!void {
    std.mem.doNotOptimizeAway(postInitUart);
    //console.register_console(@constCast(&uart_console.getConsole()));
}

fn postInitGpio() anyerror!void {
    try gpio.map_pl011_uart();
}

fn postInitRng() anyerror!void {
    std.mem.doNotOptimizeAway(postInitRng);
}

fn driverUart() anyerror!void {
    pl011_uart = bcm_pl011_uart.new(memory.mmio.PL011_UART_START);

    const driver_descriptor = driver.DeviceDriverDescriptor.new(pl011_uart.driver(), postInitUart);
    driver.driver_manager().add_driver(driver_descriptor);
}

fn driverGpio() anyerror!void {
    gpio = bcm_gpio.new(memory.mmio.GPIO_START);

    const driver_descriptor = driver.DeviceDriverDescriptor.new(gpio.driver(), postInitGpio);
    driver.driver_manager().add_driver(driver_descriptor);
}

fn driverRng() anyerror!void {
    rng = bcm_rng.new(memory.mmio.RNG_START);

    const driver_descriptor = driver.DeviceDriverDescriptor.new(rng.driver(), postInitRng);
    driver.driver_manager().add_driver(driver_descriptor);
}

pub fn init() anyerror!void {
    try driverUart();
    try driverGpio();
    try driverRng();
}
