#pragma once
#include <stddef.h>
#include <stdint.h>

#define MAX_TASKS 8
#define STACK_SIZE 1024

enum class TaskState : uint8_t {
  New = 0,
  Running = 1,
  Runnable = 2,
  Blocked = 3,
  Dying = 4,
  Idle = 5,
};

enum class TaskKind : uint8_t {
  Kernel = 0,
  User = 1,
};

enum class WaitReason : uint8_t {
  None = 0,
  Interrupt = 1,
  Timer = 2,
  EventQueue = 3,
  Sleep = 4,
  Join = 5,
  IO = 6,
};

struct Context {
  unsigned long x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30;
  unsigned long sp;
};

struct Task {
  Context context;
  unsigned long *stackPtr = nullptr;
  alignas(16) unsigned long stack[STACK_SIZE];
  void (*entry)() = nullptr;
  bool hasStarted = false;
  const char *name = nullptr;
  int id = -1; // Unique identifier for the task
  TaskState state = TaskState::New;
  TaskKind kind = TaskKind::Kernel;
  int priority = 1;
  int counter = 0;
  int cpuAffinity = 0;
  WaitReason waitReason = WaitReason::None;

  size_t stackStart() const { return reinterpret_cast<size_t>(&stack[0]); }

  size_t stackEndExclusive() const {
    return reinterpret_cast<size_t>(&stack[STACK_SIZE]);
  }
};

class TaskManager {
public:
  void init();
  bool addTask(const char *name, void (*entryPoint)(), int priority = 1,
               TaskState initialState = TaskState::Runnable,
               TaskKind kind = TaskKind::Kernel);
  void schedule();
  void yieldCurrent();
  void blockCurrent(WaitReason waitReason);
  bool wakeTask(int taskId);
  Task *current();
  const Task *current() const;
  Task *findById(int taskId);
  const Task *findById(int taskId) const;
  size_t taskCountActive() const;
  size_t runnableCount() const;
  size_t blockedCount() const;
  int currentTask = -1;

private:
  Task *pickNextTask();
  Task *idleTask();
  static bool isRunnableState(TaskState state);
  static const char *stateName(TaskState state);
  static const char *waitReasonName(WaitReason waitReason);

  Task tasks[MAX_TASKS];
  int taskCount = 0;
  int idCounter;
};

extern TaskManager taskManager;

namespace Tasks {
void yield();
void block(WaitReason waitReason);
} // namespace Tasks
