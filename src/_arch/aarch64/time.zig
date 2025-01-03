const std = @import("std");
const time = @import("../../std/time.zig");
const AtomicOrder = std.builtin.AtomicOrder;
const cpu = @import("../../cpu.zig").cpu;
const warn = @import("../../print.zig").warn;

var ARCH_TIMER_COUNTER_FREQUENCY: usize = undefined;
const NANOSEC_PER_SEC = 1_000_000_000;

pub const TimeError = error{
    DurationTooBig,
};

const GenericTimerCounterValue = struct {
    value: u64,

    pub const MAX: GenericTimerCounterValue = GenericTimerCounterValue{ .value = 1_000_000_000 - 1 };

    pub fn add(self: GenericTimerCounterValue, other: GenericTimerCounterValue) GenericTimerCounterValue {
        return GenericTimerCounterValue{ .value = self.value +% other.value };
    }

    pub fn fromDuration(duration: time.Duration) !GenericTimerCounterValue {
        if (duration.asNanos() < resolution().asNanos()) {
            return GenericTimerCounterValue{ .value = 0 };
        }

        if (duration.asNanos() > maxDuration().asNanos()) {
            return TimeError.DurationTooBig;
        }

        const duration_ns: u64 = @intCast(duration.asNanos());
        const counter_value = duration_ns * arch_timer_counter_frequency() / NANOSEC_PER_SEC;

        return GenericTimerCounterValue{ .value = counter_value };
    }

    pub fn toDuration(self: GenericTimerCounterValue) time.Duration {
        if (self.value == 0) {
            return time.Duration.zero();
        }

        const secs: u64 = self.value / arch_timer_counter_frequency();
        const sub_second_counter_value: u64 = self.value % arch_timer_counter_frequency();
        const nanos: u32 = @as(u32, @intCast(sub_second_counter_value * NANOSEC_PER_SEC / arch_timer_counter_frequency()));

        return time.Duration.new(secs, nanos);
    }
};

pub fn init() void {
    ARCH_TIMER_COUNTER_FREQUENCY = asm ("mrs %[ARCH_TIMER_COUNTER_FREQUENCY], cntfrq_el0"
        : [ARCH_TIMER_COUNTER_FREQUENCY] "=r" (-> usize),
    );
}

fn arch_timer_counter_frequency() u32 {
    return @intCast(@volatileCast(&ARCH_TIMER_COUNTER_FREQUENCY).*);
}

pub fn resolution() time.Duration {
    return (GenericTimerCounterValue{ .value = 1 }).toDuration();
}

fn maxDuration() time.Duration {
    return GenericTimerCounterValue.MAX.toDuration();
}

fn read_cntpct() GenericTimerCounterValue {
    const cntpct = asm volatile ("mrs %[cntpct], cntpct_el0"
        : [cntpct] "=r" (-> usize),
    );
    return GenericTimerCounterValue{ .value = @intCast(cntpct) };
}

pub fn uptime() time.Duration {
    return read_cntpct().toDuration();
}

pub fn spin_for(duration: time.Duration) void {
    var curr_counter_value = read_cntpct();

    const counter_value_delta = GenericTimerCounterValue.fromDuration(duration) catch |err| {
        warn("spin_for: %s. Skipping\n", .{@as([*c]const u8, @ptrCast(@errorName(err)))});
        return;
    };
    const counter_value_target = curr_counter_value.add(counter_value_delta);

    while (curr_counter_value.value < counter_value_target.value) {
        curr_counter_value = read_cntpct();
    }
}
