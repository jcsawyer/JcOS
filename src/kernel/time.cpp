#include "time.hpp"
#include <stddef.h>

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
