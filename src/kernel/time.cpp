#include "time.hpp"
#include <arch/time.hpp>
#include <container/vector.hpp>
#include <driver/driver.hpp>
#include <exceptions/asynchronous.hpp>
#include <print.hpp>
#include <synchronization.hpp>

namespace Time {

namespace {
class TimerDriverAdapter : public Driver::DeviceDriver,
                           public Exceptions::Asynchronous::IRQHandler {
public:
  const char *compatible() override { return "ARM Architectural Timer"; }

  void init() override {}

  void registerAndEnableIrqHandler(
      ::BSP::Exception::Asynchronous::IRQNumber *irqNumber) override {
    Exceptions::Asynchronous::IRQHandlerDescriptor descriptor(
        irqNumber->kind, *irqNumber, compatible(), this);

    Exceptions::Asynchronous::IRQManager *irqManager =
        Exceptions::Asynchronous::irq_manager();
    irqManager->registerHandler(descriptor);
    irqManager->enable(irqNumber);
  }

  bool handle() override {
    return TimeManager::GetInstance()->handleTimeoutIrq();
  }
};

TimerDriverAdapter g_timerDriver;
} // namespace

struct TimeManager::Impl {
  class OrderedTimeoutQueueStorage {
  public:
    void push(const Timeout &timeout) {
      inner.pushBack(timeout);

      for (size_t i = inner.size() - 1; i > 0; --i) {
        if (!(inner[i].dueTime > inner[i - 1].dueTime)) {
          break;
        }

        Timeout temp = inner[i];
        inner[i] = inner[i - 1];
        inner[i - 1] = temp;
      }
    }

    bool empty() const { return inner.empty(); }

    Time::Duration peekNextDueTime() const { return inner.back().dueTime; }

    Timeout pop() { return inner.popBack(); }

  private:
    Container::Vector<Timeout> inner;
  };

  OrderedTimeoutQueueStorage queue;
  Syncrhonization::IRQSafeNullLock<OrderedTimeoutQueueStorage> queueLock;

  Impl() : queue(), queueLock(queue) {}
};

TimeManager *TimeManager::instance = nullptr;

TimeManager::TimeManager() : impl(nullptr) {
  static Impl timeManagerImpl;
  impl = &timeManagerImpl;
}

TimeManager *TimeManager::GetInstance() {
  if (instance == nullptr) {
    static TimeManager timeManager;
    instance = &timeManager;
  }
  return instance;
}

void TimeManager::earlyInit() { Time::Arch::init(); }

Time::Duration TimeManager::resolution() const {
  return Time::Arch::resolution();
}

Time::Duration TimeManager::uptime() const { return Time::Arch::uptime(); }

void TimeManager::spinFor(const Time::Duration &duration) {
  Time::Arch::spinFor(duration);
}

void TimeManager::setTimeout(const Timeout &timeout) {
  impl->queueLock.lock([&](Impl::OrderedTimeoutQueueStorage &lockedQueue) {
    lockedQueue.push(timeout);
    Time::Arch::setTimeoutIrq(lockedQueue.peekNextDueTime());
  });
}

void TimeManager::setTimeoutOnce(const Time::Duration &delay,
                                 TimeoutCallback callback) {
  Timeout timeout{uptime() + delay, false, Time::Duration::zero(), callback};
  setTimeout(timeout);
}

void TimeManager::setTimeoutPeriodic(const Time::Duration &delay,
                                     TimeoutCallback callback) {
  Timeout timeout{uptime() + delay, true, delay, callback};
  setTimeout(timeout);
}

bool TimeManager::handleTimeoutIrq() {
  Time::Arch::concludeTimeoutIrq();

  Timeout readyTimeout{};
  bool hasReadyTimeout = false;

  impl->queueLock.lock([&](Impl::OrderedTimeoutQueueStorage &lockedQueue) {
    if (lockedQueue.empty()) {
      return;
    }

    if (lockedQueue.peekNextDueTime() > uptime()) {
      return;
    }

    readyTimeout = lockedQueue.pop();
    if (readyTimeout.periodic) {
      readyTimeout.refresh();
    }

    hasReadyTimeout = true;
  });

  if (!hasReadyTimeout) {
    warn("Spurious timeout IRQ");
    return true;
  }

  readyTimeout.callback();

  impl->queueLock.lock([&](Impl::OrderedTimeoutQueueStorage &lockedQueue) {
    if (readyTimeout.periodic) {
      lockedQueue.push(readyTimeout);
    }

    if (!lockedQueue.empty()) {
      Time::Arch::setTimeoutIrq(lockedQueue.peekNextDueTime());
    }
  });

  return true;
}

void registerTimerDriver() {
  Driver::driverManager().addDriver(Driver::DeviceDriverDescriptor(
      &g_timerDriver, nullptr, Time::Arch::timeoutIrq()));
}

} // namespace Time
