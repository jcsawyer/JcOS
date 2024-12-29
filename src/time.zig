pub fn init() void {
    cntfrq = asm ("mrs %[cntfrq], cntfrq_el0"
        : [cntfrq] "=r" (-> usize),
    );
    if (cntfrq == 0) {
        cntfrq = 1 * 1000 * 1000;
    }
    cntfrq_f32 = @floatFromInt(cntfrq);
    update();
}

pub fn update() void {
    cntpct = asm ("mrs %[cntpct], cntpct_el0"
        : [cntpct] "=r" (-> usize),
    );
    seconds = @as(f32, @floatCast(@as(f32, @floatFromInt(cntpct)))) / cntfrq_f32;
    milliseconds = cntpct / (cntfrq / 1000);
}

pub fn sleep(duration: f32) void {
    update();
    const start = seconds;
    while (seconds - start < duration) {
        update();
    }
}

pub fn uptime() f32 {
    update();
    return seconds;
}

pub var seconds: f32 = undefined;
pub var milliseconds: usize = undefined;

var cntfrq: usize = undefined;
var cntfrq_f32: f32 = undefined;
var cntpct: usize = undefined;
