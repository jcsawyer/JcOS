const std = @import("std");
const bsp = @import("bsp.zig").board;
const console = bsp.console;

const NUM_DRIVERS: usize = @intCast(5);
pub const DeviceDriverPostInitCallback = fn () anyerror!void;

var driverManager = DriverManager{};

pub const DeviceDriver = struct {
    context: *anyopaque,
    compatible: *const fn () []const u8,
    init: *const fn () anyerror!void = &defaultInit,
};

fn defaultInit() anyerror!void {
    // Default implementation does nothing
    std.mem.doNotOptimizeAway(defaultInit);
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
        std.mem.doNotOptimizeAway(add_driver);
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
        for (self.drivers[0..self.next_index]) |driver| {
            const compatible = driver.device_driver.compatible();
            console.printLn("\t- {s}", .{compatible});
        }
    }
};

pub fn driver_manager() *DriverManager {
    return &driverManager;
}
