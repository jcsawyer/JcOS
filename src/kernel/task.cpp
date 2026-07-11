#include "task.hpp"
#include <print.hpp>

TaskManager taskManager;

extern "C" void switch_context(unsigned long *oldStackPtr,
                               unsigned long *newStackPtr);
extern "C" void enter_task(void (*entry)(void), unsigned long *stackTop);

void TaskManager::init() {
  taskCount = 0;
  currentTask = -1;
  idCounter = 1000;
}

bool TaskManager::isRunnableState(TaskState state) {
  return state == TaskState::Runnable || state == TaskState::Running;
}

const char *TaskManager::stateName(TaskState state) {
  switch (state) {
  case TaskState::New:
    return "new";
  case TaskState::Running:
    return "running";
  case TaskState::Runnable:
    return "runnable";
  case TaskState::Blocked:
    return "blocked";
  case TaskState::Dying:
    return "dying";
  case TaskState::Idle:
    return "idle";
  default:
    return "unknown";
  }
}

const char *TaskManager::waitReasonName(WaitReason waitReason) {
  switch (waitReason) {
  case WaitReason::None:
    return "none";
  case WaitReason::Interrupt:
    return "interrupt";
  case WaitReason::Timer:
    return "timer";
  case WaitReason::EventQueue:
    return "event-queue";
  case WaitReason::Sleep:
    return "sleep";
  case WaitReason::Join:
    return "join";
  case WaitReason::IO:
    return "io";
  default:
    return "unknown";
  }
}

bool TaskManager::addTask(const char *name, void (*entry)(), int priority,
                          TaskState initialState, TaskKind kind) {

  if (taskCount >= MAX_TASKS) {
    warn("Task creation failed for %s: task table full", name);
    return false;
  }

  Task &t = tasks[taskCount];
  t.stackPtr = nullptr;
  t.entry = nullptr;
  t.hasStarted = false;
  t.name = name;
  t.id = ++idCounter;
  t.state = initialState;
  t.kind = kind;
  t.priority = priority > 0 ? priority : 1;
  t.counter = t.priority;
  t.cpuAffinity = 0;
  t.waitReason = WaitReason::None;

  // Initialize context
  for (int i = 0; i < 12; ++i)
    (&t.context.x19)[i] = 0;

  t.context.x30 = reinterpret_cast<unsigned long>(entry); // LR = entry point

  // Stack pointer: top of stack, aligned 16 bytes
  size_t stackTopAddress = reinterpret_cast<size_t>(&t.stack[STACK_SIZE]);
  stackTopAddress &= ~static_cast<size_t>(0xFul);
  t.context.sp = static_cast<unsigned long>(stackTopAddress);

  // Point stackPtr to context for switch_context
  t.stackPtr = reinterpret_cast<unsigned long *>(&t.context);
  t.entry = entry;
  taskCount++;

  info("Task %p (TID %d) created: %s state=%s priority=%d wait=%s", entry, t.id,
       name, stateName(t.state), t.priority, waitReasonName(t.waitReason));
  return true;
}

Task *TaskManager::current() {
  if (currentTask >= 0)
    return &tasks[currentTask];
  return nullptr;
}

const Task *TaskManager::current() const {
  if (currentTask >= 0)
    return &tasks[currentTask];
  return nullptr;
}

Task *TaskManager::findById(int taskId) {
  for (int i = 0; i < taskCount; ++i) {
    if (tasks[i].id == taskId) {
      return &tasks[i];
    }
  }
  return nullptr;
}

const Task *TaskManager::findById(int taskId) const {
  for (int i = 0; i < taskCount; ++i) {
    if (tasks[i].id == taskId) {
      return &tasks[i];
    }
  }
  return nullptr;
}

size_t TaskManager::taskCountActive() const {
  return static_cast<size_t>(taskCount);
}

size_t TaskManager::runnableCount() const {
  size_t count = 0;
  for (int i = 0; i < taskCount; ++i) {
    if (isRunnableState(tasks[i].state)) {
      ++count;
    }
  }
  return count;
}

size_t TaskManager::blockedCount() const {
  size_t count = 0;
  for (int i = 0; i < taskCount; ++i) {
    if (tasks[i].state == TaskState::Blocked) {
      ++count;
    }
  }
  return count;
}

Task *TaskManager::idleTask() {
  for (int i = 0; i < taskCount; ++i) {
    if (tasks[i].state == TaskState::Idle) {
      return &tasks[i];
    }
  }
  return nullptr;
}

Task *TaskManager::pickNextTask() {
  Task *selected = nullptr;
  int bestCounter = -1;

  while (selected == nullptr) {
    for (int i = 0; i < taskCount; ++i) {
      Task &task = tasks[i];
      if (!isRunnableState(task.state) || task.counter <= 0) {
        continue;
      }

      if (selected == nullptr || task.counter > bestCounter) {
        selected = &task;
        bestCounter = task.counter;
      }
    }

    if (selected != nullptr) {
      return selected;
    }

    bool recalculated = false;
    for (int i = 0; i < taskCount; ++i) {
      Task &task = tasks[i];
      if (!isRunnableState(task.state)) {
        continue;
      }
      task.counter = (task.counter >> 1) + task.priority;
      recalculated = true;
    }

    if (!recalculated) {
      return idleTask();
    }
  }

  return selected;
}

void TaskManager::schedule() {
  if (taskCount == 0) {
    return;
  }

  Task *old = current();
  if (old != nullptr && old->state == TaskState::Running) {
    old->state = TaskState::Runnable;
  }

  Task *nextTask = pickNextTask();
  if (nextTask == nullptr) {
    return;
  }

  currentTask = static_cast<int>(nextTask - tasks);
  nextTask->state =
      nextTask->state == TaskState::Idle ? TaskState::Idle : TaskState::Running;
  if (nextTask->state == TaskState::Running && nextTask->counter > 0) {
    --nextTask->counter;
  }

  if (old == nextTask) {
    return;
  }

  if (!nextTask->hasStarted) {
    nextTask->hasStarted = true;

    enter_task(nextTask->entry,
               reinterpret_cast<unsigned long *>(nextTask->context.sp));
  } else {
    if (old != nullptr) {
      // switch_context never returns, switches context to next task
      switch_context(old->stackPtr, nextTask->stackPtr);
    }
  }
}

void TaskManager::yieldCurrent() {
  Task *task = current();
  if (task != nullptr && task->state != TaskState::Idle) {
    task->counter = 0;
  }
  schedule();
}

void TaskManager::blockCurrent(WaitReason waitReason) {
  Task *task = current();
  if (task == nullptr || task->state == TaskState::Idle) {
    return;
  }

  task->state = TaskState::Blocked;
  task->waitReason = waitReason;
  task->counter = 0;
  info("Task %s (TID %d) blocked: reason=%s", task->name, task->id,
       waitReasonName(waitReason));
  schedule();
}

bool TaskManager::wakeTask(int taskId) {
  Task *task = findById(taskId);
  if (task == nullptr || task->state != TaskState::Blocked) {
    return false;
  }

  task->state = TaskState::Runnable;
  task->waitReason = WaitReason::None;
  if (task->counter <= 0) {
    task->counter = task->priority;
  }

  info("Task %s (TID %d) woke: priority=%d counter=%d", task->name, task->id,
       task->priority, task->counter);
  return true;
}

namespace Tasks {

void yield() { taskManager.yieldCurrent(); }

void block(WaitReason waitReason) { taskManager.blockCurrent(waitReason); }

} // namespace Tasks
