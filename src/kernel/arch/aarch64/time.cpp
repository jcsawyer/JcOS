#include "../time.hpp"
#include "../../bsp/raspberrypi/exception/asynchronous.hpp"
#include "../../print.hpp"
#include "../cpu.hpp"

extern "C" {
__attribute__((used, visibility("default"))) extern uint64_t
    ARCH_TIMER_COUNTER_FREQUENCY asm("ARCH_TIMER_COUNTER_FREQUENCY") = 0;
}
namespace Time {
namespace Arch {

GenericTimerCounterValue
GenericTimerCounterValue::fromDuration(const Duration &duration,
                                       TimeError &error) {
  if (duration.as_nanos() < resolution().as_nanos()) {
    return GenericTimerCounterValue(0);
  }
  if (duration.as_nanos() > Duration::max().as_nanos()) {
    error = TimeError::DurationTooBig;
    return GenericTimerCounterValue(0);
  }

  const uint64_t duration_ns = duration.as_nanos();
  const uint64_t counter_value =
      duration_ns * archTimerCounterFrequency() / NANOSEC_PER_SEC;
  return GenericTimerCounterValue(counter_value);
}

Duration GenericTimerCounterValue::toDuration() const {
  if (value == 0) {
    return Duration::zero();
  }

  const uint64_t secs = value / archTimerCounterFrequency();
  const uint64_t sub_second_counter_value = value % archTimerCounterFrequency();
  const uint32_t nanos = static_cast<uint32_t>(
      sub_second_counter_value * NANOSEC_PER_SEC / archTimerCounterFrequency());

  return Duration(secs, nanos);
}

uint32_t GenericTimerCounterValue::archTimerCounterFrequency() {
  __atomic_load_n(&ARCH_TIMER_COUNTER_FREQUENCY, __ATOMIC_SEQ_CST);
  return ARCH_TIMER_COUNTER_FREQUENCY;
}

void init() {
  asm volatile("mrs %0, cntfrq_el0" : "=r"(ARCH_TIMER_COUNTER_FREQUENCY));
}

Duration resolution() { return GenericTimerCounterValue(1).toDuration(); }

Duration maxDuration() { return GenericTimerCounterValue::MAX.toDuration(); }

GenericTimerCounterValue readCntpct() {
  uint64_t cntpct;
  asm volatile("mrs %0, cntpct_el0" : "=r"(cntpct));

  return GenericTimerCounterValue(cntpct);
}

Duration uptime() { return readCntpct().toDuration(); }

void spinFor(const Duration &duration) {
  const GenericTimerCounterValue curr_counter_value = readCntpct();
  TimeError error = TimeError::None;

  const GenericTimerCounterValue counter_value_delta =
      GenericTimerCounterValue::fromDuration(duration, error);
  if (error == TimeError::DurationTooBig) {
    warn("spinFor: duration too big. Skipping.");
    return;
  }

  const GenericTimerCounterValue counter_value_target =
      curr_counter_value.add(counter_value_delta);

  while (readCntpct().value < counter_value_target.value) {
    CPU::nop();
  }
}

::BSP::Exception::Asynchronous::IRQNumber *timeoutIrq() {
#if BOARD == bsp_rpi3
  return Driver::BSP::RaspberryPi::ARM_NS_PHYSICAL_TIMER();
#else
#error "Tutorial 20 timer callbacks currently support bsp_rpi3 only"
#endif
}

void setTimeoutIrq(const Duration &dueTime) {
  TimeError error = TimeError::None;
  const GenericTimerCounterValue counter_value_target =
      GenericTimerCounterValue::fromDuration(dueTime, error);

  if (error == TimeError::DurationTooBig) {
    warn("setTimeoutIrq: duration too big. Skipping.");
    return;
  }

  asm volatile("msr cntp_cval_el0, %0" ::"r"(counter_value_target.value));
  asm volatile("msr cntp_ctl_el0, %0" ::"r"(1ul));
  asm volatile("isb");
}

void concludeTimeoutIrq() {
  asm volatile("msr cntp_ctl_el0, %0" ::"r"(0ul));
  asm volatile("isb");
}
} // namespace Arch
} // namespace Time
