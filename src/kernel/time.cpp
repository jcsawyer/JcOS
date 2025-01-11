#include "time.hpp"

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
