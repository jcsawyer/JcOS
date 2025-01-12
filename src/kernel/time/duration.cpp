#include "duration.hpp"

namespace Time {

// Constructors
Duration::Duration(const uint64_t secs, const uint32_t nanos)
    : seconds_(secs), nanoseconds_(nanos) {
  if (nanoseconds_ >= NANOS_PER_SEC) {
    seconds_ += nanoseconds_ / NANOS_PER_SEC;
    nanoseconds_ %= NANOS_PER_SEC;
  }
}

// Static factory methods
Duration Duration::zero() { return Duration(0, 0); }

Duration Duration::max() { return Duration(INT32_MAX, NANOS_PER_SEC - 1); }

Duration Duration::from_secs(const uint64_t secs) { return Duration(secs, 0); }

Duration Duration::from_millis(const uint64_t millis) {
  return Duration(millis / 1'000, millis % 1'000 * 1'000'000);
}

Duration Duration::from_micros(const uint64_t micros) {
  return Duration(micros / 1'000'000, micros % 1'000'000 * 1'000);
}

Duration Duration::from_nanos(const uint64_t nanos) {
  return Duration(nanos / NANOS_PER_SEC, nanos % NANOS_PER_SEC);
}

// Accessors
uint64_t Duration::as_secs() const { return seconds_; }

__uint128_t Duration::as_millis() const {
  return seconds_ * 1'000 + nanoseconds_ / 1'000'000;
}

__uint128_t Duration::as_micros() const {
  return seconds_ * 1'000'000 + nanoseconds_ / 1'000;
}

__uint128_t Duration::as_nanos() const {
  return seconds_ * NANOS_PER_SEC + nanoseconds_;
}

uint32_t Duration::subsec_nanos() const { return nanoseconds_; }

uint32_t Duration::subsec_micros() const { return nanoseconds_ / 1'000; }

uint32_t Duration::subsec_millis() const { return nanoseconds_ / 1'000'000; }

// Comparison operators
bool Duration::operator==(const Duration &other) const {
  return seconds_ == other.seconds_ && nanoseconds_ == other.nanoseconds_;
}

bool Duration::operator!=(const Duration &other) const {
  return !(*this == other);
}

bool Duration::operator<(const Duration &other) const {
  return seconds_ < other.seconds_ ||
         (seconds_ == other.seconds_ && nanoseconds_ < other.nanoseconds_);
}

bool Duration::operator<=(const Duration &other) const {
  return *this < other || *this == other;
}

bool Duration::operator>(const Duration &other) const {
  return !(*this <= other);
}

bool Duration::operator>=(const Duration &other) const {
  return !(*this < other);
}

// Arithmetic operators
Duration Duration::operator+(const Duration &other) const {
  return Duration(seconds_ + other.seconds_, nanoseconds_ + other.nanoseconds_);
}

Duration Duration::operator+=(const Duration &other) {
  seconds_ += other.seconds_;
  nanoseconds_ += other.nanoseconds_;
}

Duration Duration::operator-(const Duration &other) const {
  if (*this < other) {
    return zero(); // Prevent negative durations
  }

  uint64_t secs = seconds_ - other.seconds_;
  int64_t nanos = static_cast<int64_t>(nanoseconds_) -
                  static_cast<int64_t>(other.nanoseconds_);

  if (nanos < 0) {
    --secs;
    nanos += NANOS_PER_SEC;
  }

  return Duration(secs, static_cast<uint32_t>(nanos));
}
} // namespace Time
