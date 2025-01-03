const std = @import("std");
const console = @import("print.zig");

const NUM_DRIVERS: usize = @intCast(5);
pub const DeviceDriverPostInitCallback = fn () anyerror!void;

var driverManager = DriverManager{};

pub const DeviceDriver = struct {
    ctx: *anyopaque = undefined,
    vtable: *const VTable,

    pub const VTable = struct {
        compatible: *const fn (ctx: *anyopaque) []const u8,
        init: *const fn (ctx: *anyopaque) anyerror!void = &defaultInit,
    };

    pub fn compatible(self: DeviceDriver) []const u8 {
        return self.vtable.compatible(self.ctx);
    }

    pub fn init(self: DeviceDriver) anyerror!void {
        return self.vtable.init(self.ctx);
    }
};

fn defaultInit(ctx: *anyopaque) anyerror!void {
    // Default implementation does nothing
    _ = ctx;
}

pub const DeviceDriverDescriptor = struct {
    device_driver: DeviceDriver = undefined,
    post_init_callback: ?*const DeviceDriverPostInitCallback = undefined,

    pub fn new(device_driver: DeviceDriver, post_init_callback: ?DeviceDriverPostInitCallback) DeviceDriverDescriptor {
        return .{
            .device_driver = device_driver,
            .post_init_callback = post_init_callback orelse null,
        };
    }
};

pub const DriverManager = struct {
    drivers: [NUM_DRIVERS]DeviceDriverDescriptor = undefined,
    next_index: usize = 0,

    pub fn add_driver(self: *DriverManager, driver: DeviceDriverDescriptor) void {
        if (self.next_index >= NUM_DRIVERS) {
            return;
        }
        self.drivers[self.next_index] = driver;
        self.next_index += 1;
    }

    pub fn init(self: *DriverManager) anyerror!void {
        for (self.drivers[0..self.next_index]) |driver| {
            try driver.device_driver.init();

            if (driver.post_init_callback) |callback| {
                try callback();
            }
        }
    }

    pub fn print_drivers(self: *DriverManager) void {
        for (self.drivers[0..self.next_index], 0..) |driver, i| {
            const compatible = driver.device_driver.compatible();
            console.info("\t %d. %s", .{ i + 1, @as([*:0]const u8, @ptrCast(compatible)) });
        }
    }
};

pub fn driver_manager() *DriverManager {
    return &driverManager;
}
