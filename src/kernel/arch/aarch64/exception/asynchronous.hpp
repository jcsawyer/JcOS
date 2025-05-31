#pragma once
#include <stdint.h>

namespace Exception {
namespace DAIF {
const unsigned long D = (1ULL << 9); // Debug
const unsigned long A = (1ULL << 8); // SError
const unsigned long I = (1ULL << 7); // IRQ
const unsigned long F = (1ULL << 6); // FIQ

// Function to check if a DAIF field is set
inline bool is_set(unsigned long field) {
  unsigned long daif_value;
  asm volatile("mrs %0, DAIF" : "=r"(daif_value)); // Read DAIF register
  return (daif_value & field) != 0;
}
} // namespace DAIF

// DAIF Field Interface (equivalent to Rust's trait)
class DaifField {
public:
  virtual unsigned long daif_field() const = 0;
};

// Specific DAIF Fields
class Debug : public DaifField {
public:
  unsigned long daif_field() const override { return DAIF::D; }
};

class SError : public DaifField {
public:
  unsigned long daif_field() const override { return DAIF::A; }
};

class IRQ : public DaifField {
public:
  unsigned long daif_field() const override { return DAIF::I; }
};

class FIQ : public DaifField {
public:
  unsigned long daif_field() const override { return DAIF::F; }
};

// Function to check if a DAIF field is masked
template <typename T> bool is_masked() {
  T field;
  return DAIF::is_set(field.daif_field());
}

namespace Asynchronous {
unsigned long localIrqMaskSave();

void localIrqRestore(unsigned long flags);

void localIrqMask();

void localIrqUnmask();

bool isLocalIrqMasked();
} // namespace Asynchronous
} // namespace Exception