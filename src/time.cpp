#include "time.hpp"
#include "std/stddef.h"

namespace Time {
TimeManager *TimeManager::instance = nullptr;

TimeManager *TimeManager::GetInstance() {
  if (instance == nullptr) {
    static TimeManager timeManager;
    instance = &timeManager;
  }
  return instance;
}
} // namespace Time
