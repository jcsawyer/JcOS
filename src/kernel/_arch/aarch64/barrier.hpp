#pragma once

namespace Barrier {
/// Data Synchronization Barrier with "sy" scope
inline void dsb_sy() { asm volatile("dsb sy" ::: "memory"); }

/// Data Memory Barrier with "sy" scope
inline void dmb_sy() { asm volatile("dmb sy" ::: "memory"); }

/// Instruction Synchronization Barrier
inline void isb() { asm volatile("isb" ::: "memory"); }
} // namespace Barrier
