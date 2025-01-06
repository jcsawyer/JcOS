#include "../time.hpp"
#include "../../print.hpp"
#include "../cpu.hpp"

extern "C" {
__attribute__((used, visibility("default"))) extern uint64_t
    ARCH_TIMER_COUNTER_FREQUENCY asm("ARCH_TIMER_COUNTER_FREQUENCY") = 0;
}
namespace Time {
namespace Arch {

Time::Arch::GenericTimerCounterValue
Time::Arch::GenericTimerCounterValue::fromDuration(const Duration &duration,
                                                   TimeError &error) {
  if (duration.as_nanos() < resolution().as_nanos()) {
    return GenericTimerCounterValue(0);
  }
  if (duration.as_nanos() > Time::Duration::max().as_nanos()) {
    error = TimeError::DurationTooBig;
    return GenericTimerCounterValue(0);
  }

  uint64_t duration_ns = duration.as_nanos();
  uint64_t counter_value =
      (duration_ns * archTimerCounterFrequency()) / NANOSEC_PER_SEC;
  return GenericTimerCounterValue(counter_value);
}

Time::Duration Time::Arch::GenericTimerCounterValue::toDuration() const {
  if (value == 0) {
    return Duration::zero();
  }

  uint64_t secs = value / archTimerCounterFrequency();
  uint64_t sub_second_counter_value = value % archTimerCounterFrequency();
  uint32_t nanos =
      static_cast<uint32_t>((sub_second_counter_value * NANOSEC_PER_SEC) /
                            archTimerCounterFrequency());

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
  GenericTimerCounterValue curr_counter_value = readCntpct();
  TimeError error;

  GenericTimerCounterValue counter_value_delta =
      GenericTimerCounterValue::fromDuration(duration, error);
  if (error == TimeError::DurationTooBig) {
    warn("spinFor: %d. Skipping.");
    return;
  }

  GenericTimerCounterValue counter_value_target =
      curr_counter_value.add(counter_value_delta);

  while (readCntpct().value < counter_value_target.value) {
    CPU::nop();
  }
}
} // namespace Arch
} // namespace Time
