const std = @import("std");
const config = @import("config");
const cpu = @import("../../../cpu.zig");
const DeviceDriver = @import("../../../driver.zig").DeviceDriver;

// Make an educated guess for a good delay value (Sequence described in the BCM2837
// peripherals PDF).
//
// - According to Wikipedia, the fastest RPi4 clocks around 1.5 GHz.
// - The Linux 2837 GPIO driver waits 1 µs between the steps.
//
// So lets try to be on the safe side and default to 2000 cycles, which would equal 1 µs
// would the CPU be clocked at 2 GHz.
const DELAY: usize = 2000;

// GPIO function select 1
const GPFSEL1 = enum(u32) {
    Input = 0x0b000,
    Output = 0x0b001,
    AltFunc0 = 0x0b100, // PL011 UART RX
};

// GPIO pull-up/down register
// BCM2837 only
// Controls the actuation of the internal pull-up/down control line to ALL the GPIO pins.
const GPPUD_PUD = enum(u32) {
    Off = 0,
    PullDown = 0b10,
    PullUp = 0b01,
};

// GPIO pull-up/down clock register 0
const GPPUCLK0 = enum(u32) {
    NoEffect = 0,
    AssertClock = 1,
};

// GPIO pull-up/down register 0
const GPIO_PUP_PDN_CNTRL_REG0 = enum(u32) {
    NoResistor = 0b00,
    PullUp = 0b01,
};

const RegisterBlock = struct {
    _reserved1: u32, // (0x00)
    GPFSEL1: *volatile u32, // (0x04)
    _reserved2: u32, // (0x08) - Filler to align to 0x94
    _reserved2_fill: u32, // Filler for alignment
    _reserved2_fill2: u32, // Filler for alignment
    _reserved2_fill3: u32, // Filler for alignment
    _reserved2_fill4: u32, // Filler for alignment
    _reserved2_fill5: u32, // Filler for alignment
    _reserved2_fill6: u32, // Filler for alignment
    _reserved2_fill7: u32, // Filler for alignment
    _reserved2_fill8: u32, // Filler for alignment
    _reserved2_fill9: u32, // Filler for alignment
    _reserved2_fill10: u32, // Filler for alignment
    _reserved2_fill11: u32, // Filler for alignment
    _reserved2_fill12: u32, // Filler for alignment
    GPPUD: *volatile u32, // (0x94)
    GPPUDCLK0: *volatile u32, // (0x98)
    _reserved3: u32, // (0x9C)
    _reserved3_fill: u32, // Filler for alignment
    GPIO_PUP_PDN_CNTRL_REG0: *volatile u32, // (0xE4)
};

const MMIODerefWarpper = struct {
    registers: *const volatile RegisterBlock,

    pub fn new(mmio_start_addr: usize) MMIODerefWarpper {
        return .{
            .registers = @as(*const RegisterBlock, @ptrFromInt(mmio_start_addr)),
        };
    }
};

const GPIOInner = struct {
    registers: MMIODerefWarpper,

    pub fn new(mmio_start_addr: usize) GPIOInner {
        return .{
            .registers = MMIODerefWarpper.new(mmio_start_addr),
        };
    }

    pub fn disable_pud_14_15_bcm2837(self: *GPIOInner) void {
        // Disable pull-up/down for GPIO 14 & 15 (raspi3)
        self.registers.registers.GPPUD.* = @intFromEnum(GPPUD_PUD.Off);
        cpu.cpu.spin_for_cycles(DELAY);

        self.registers.registers.GPPUDCLK0.* = (@intFromEnum(GPPUCLK0.AssertClock) << 14) | (@intFromEnum(GPPUCLK0.AssertClock) << 15);
        cpu.cpu.spin_for_cycles(DELAY);

        self.registers.registers.GPPUD.* = @intFromEnum(GPPUD_PUD.Off);
        self.registers.registers.GPPUDCLK0.* = 0;
    }

    pub fn disable_pud_14_15_bcm2711(self: *GPIOInner) void {
        // Disable pull-up/down for GPIO 14 & 15 (raspi4)
        self.registers.registers.GPIO_PUP_PDN_CNTRL_REG0.* = (@intFromEnum(GPIO_PUP_PDN_CNTRL_REG0.NoResistor) << 30) | (@intFromEnum(GPIO_PUP_PDN_CNTRL_REG0.NoResistor) << 28);
    }

    pub fn map_pl011_uart(self: *GPIOInner) anyerror!void {
        // Set GPIO 14 and 15 to alternative function 0 (PL011 UART RX/TX)
        self.registers.registers.GPFSEL1.* = (@intFromEnum(GPFSEL1.AltFunc0) << 12) | (@intFromEnum(GPFSEL1.AltFunc0) << 15);

        if (std.mem.eql(u8, config.board, "bsp_rpi3")) {
            self.disable_pud_14_15_bcm2837();
        } else if (std.mem.eql(u8, config.board, "bsp_rpi4")) {
            self.disable_pud_14_15_bcm2711();
        } else {
            @panic("Cannot map pl011 uart for unknown board");
        }
    }
};

pub const GPIO = struct {
    const compatible_str: []const u8 = "BCM GPIUO";
    inner: GPIOInner,

    pub fn new(mmio_start_addr: usize) GPIO {
        return .{
            .inner = GPIOInner.new(mmio_start_addr),
        };
    }

    pub fn compatible(_: *anyopaque) []const u8 {
        return compatible_str;
    }

    pub fn map_pl011_uart(self: *GPIO) anyerror!void {
        try self.inner.map_pl011_uart();
    }

    const vtable = DeviceDriver.VTable{
        .compatible = GPIO.compatible,
    };

    pub fn driver(gpio: *GPIO) DeviceDriver {
        return DeviceDriver{
            .ctx = @ptrCast(gpio),
            .vtable = &GPIO.vtable,
        };
    }
};
