#pragma once
#include "../../../std/stdint.h"

namespace Exception {
namespace DAIF {
const uint64_t D = (1ULL << 9); // Debug
const uint64_t A = (1ULL << 8); // SError
const uint64_t I = (1ULL << 7); // IRQ
const uint64_t F = (1ULL << 6); // FIQ

// Function to check if a DAIF field is set
inline bool is_set(uint64_t field) {
  uint64_t daif_value;
  asm volatile("mrs %0, DAIF" : "=r"(daif_value)); // Read DAIF register
  return (daif_value & field) != 0;
}
} // namespace DAIF

// DAIF Field Interface (equivalent to Rust's trait)
class DaifField {
public:
  virtual uint64_t daif_field() const = 0;
};

// Specific DAIF Fields
class Debug : public DaifField {
public:
  uint64_t daif_field() const override { return DAIF::D; }
};

class SError : public DaifField {
public:
  uint64_t daif_field() const override { return DAIF::A; }
};

class IRQ : public DaifField {
public:
  uint64_t daif_field() const override { return DAIF::I; }
};

class FIQ : public DaifField {
public:
  uint64_t daif_field() const override { return DAIF::F; }
};

// Function to check if a DAIF field is masked
template <typename T> bool is_masked() {
  T field;
  return DAIF::is_set(field.daif_field());
}

void print_state();

} // namespace Exception
