const std = @import("std");

pub const Duration = struct {
    secs: u64,
    nanos: u32, // Always 0 <= nanos < NANOS_PER_SEC

    pub const NANOS_PER_SEC = 1_000_000_000;
    pub const NANOS_PER_MICRO = 1_000;
    pub const MILLIS_PER_SEC = 1_000;
    pub const MICROS_PER_SEC = 1_000_000;
    pub const MAX = Duration{ .secs = @as(u64, std.math.maxInt(u64)), .nanos = NANOS_PER_SEC - 1 };
    pub const ZERO = Duration{ .secs = 0, .nanos = 0 };

    pub fn new(secs: u64, nanos: u32) Duration {
        if (nanos < NANOS_PER_SEC) {
            return Duration{ .secs = secs, .nanos = nanos };
        } else {
            const extra_secs = nanos / NANOS_PER_SEC;
            return Duration{ .secs = secs + @as(u64, extra_secs), .nanos = nanos % NANOS_PER_SEC };
        }
    }

    pub fn zero() Duration {
        return ZERO;
    }

    pub fn fromSecs(secs: u64) Duration {
        return Duration{ .secs = secs, .nanos = 0 };
    }

    pub fn fromMillis(millis: u64) Duration {
        const secs = millis / MILLIS_PER_SEC;
        const nanos = (millis % MILLIS_PER_SEC) * 1_000_000;
        return Duration{ .secs = secs, .nanos = @as(u32, nanos) };
    }

    pub fn fromMicros(micros: u64) Duration {
        const secs = micros / MICROS_PER_SEC;
        const nanos = (micros % MICROS_PER_SEC) * 1_000;
        return Duration{ .secs = secs, .nanos = @as(u32, nanos) };
    }

    pub fn fromNanos(nanos: u64) Duration {
        const secs = nanos / NANOS_PER_SEC;
        const subsec_in_nanos = nanos % NANOS_PER_SEC;
        return Duration{ .secs = secs, .nanos = @as(u32, @intCast(subsec_in_nanos)) };
    }

    pub fn asSecs(self: Duration) u64 {
        return self.secs;
    }

    pub fn asMillis(self: Duration) u128 {
        return @as(u128, self.secs) * @as(u128, MILLIS_PER_SEC) + @as(u128, self.nanos) / 1_000_000;
    }

    pub fn asMicros(self: Duration) u128 {
        return @as(u128, self.secs) * @as(u128, MICROS_PER_SEC) + @as(u128, self.nanos) / 1_000;
    }

    pub fn asNanos(self: Duration) u128 {
        return @as(u128, self.secs) * @as(u128, NANOS_PER_SEC) + @as(u128, self.nanos);
    }

    pub fn subsec_millis(self: Duration) u32 {
        return self.nanos / NANOS_PER_SEC;
    }

    pub fn subsec_micros(self: Duration) u32 {
        return self.nanos / NANOS_PER_MICRO;
    }

    pub fn subsec_nanos(self: Duration) u32 {
        return self.nanos;
    }

    pub fn isZero(self: Duration) bool {
        return self.secs == 0 and self.nanos == 0;
    }

    pub fn checkedAdd(self: Duration, other: Duration) ?Duration {
        const secs_sum = self.secs + other.secs;
        const nanos_sum = self.nanos + other.nanos;
        if (nanos_sum >= NANOS_PER_SEC) {
            if (secs_sum == std.math.maxInt(u64)) return null;
            return Duration{
                .secs = secs_sum + 1,
                .nanos = nanos_sum - NANOS_PER_SEC,
            };
        }
        return Duration{ .secs = secs_sum, .nanos = nanos_sum };
    }

    pub fn checkedSub(self: Duration, other: Duration) ?Duration {
        if (self.secs < other.secs or (self.secs == other.secs and self.nanos < other.nanos)) {
            return null;
        }
        const secs_diff = self.secs - other.secs;
        const nanos_diff = if (self.nanos >= other.nanos) self.nanos - other.nanos else NANOS_PER_SEC + self.nanos - other.nanos;
        return Duration{ .secs = secs_diff, .nanos = nanos_diff };
    }

    pub fn saturatingAdd(self: Duration, other: Duration) Duration {
        return self.checkedAdd(other) orelse MAX;
    }

    pub fn saturatingSub(self: Duration, other: Duration) Duration {
        return self.checkedSub(other) orelse ZERO;
    }

    pub fn toString(self: Duration) []const u8 {
        return std.fmt.allocPrint("{d}.{d}s", .{ self.secs, self.nanos });
    }
};
