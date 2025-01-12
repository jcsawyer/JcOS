#pragma once

#include <stdint.h>

namespace Time {
class Duration {
public:
  // Constants
  static constexpr uint64_t NANOS_PER_SEC = 1000000000;

  // Constructors
  Duration(uint64_t secs, uint32_t nanos);

  // Static factory methods
  static Duration zero();
  static Duration max();
  static Duration from_secs(uint64_t secs);
  static Duration from_millis(uint64_t millis);
  static Duration from_micros(uint64_t micros);
  static Duration from_nanos(uint64_t nanos);

  // Accessors
  uint64_t as_secs() const;
  __uint128_t as_millis() const;
  __uint128_t as_micros() const;
  __uint128_t as_nanos() const;
  uint32_t subsec_nanos() const;
  uint32_t subsec_micros() const;
  uint32_t subsec_millis() const;

  // Comparison operators
  bool operator==(const Duration &other) const;
  bool operator!=(const Duration &other) const;
  bool operator<(const Duration &other) const;
  bool operator<=(const Duration &other) const;
  bool operator>(const Duration &other) const;
  bool operator>=(const Duration &other) const;

  // Arithmetic operators
  Duration operator+(const Duration &other) const;
  Duration operator+=(const Duration &other);
  Duration operator-(const Duration &other) const;

private:
  uint64_t seconds_;
  uint32_t nanoseconds_;
};
} // namespace Time
