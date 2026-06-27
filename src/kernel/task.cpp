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

void TaskManager::addTask(const char *name, void (*entry)()) {

  if (taskCount >= MAX_TASKS)
    return;

  Task &t = tasks[taskCount];

  t.name = name;
  t.id = ++idCounter;

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

  info("Task %p (TID %d) created: %s", entry, t.id, name);
}

Task *TaskManager::current() {
  if (currentTask >= 0)
    return &tasks[currentTask];
  return nullptr;
}

void TaskManager::schedule() {
  if (taskCount == 0)
    return; // no tasks

  int oldTaskIndex = currentTask;
  currentTask = (currentTask + 1) % taskCount;

  Task *old = (oldTaskIndex >= 0) ? &tasks[oldTaskIndex] : nullptr;
  Task *nextTask = &tasks[currentTask];

  if (old == nextTask) {
    return;
  }

  if (!nextTask->hasStarted) {
    nextTask->hasStarted = true;

    enter_task(nextTask->entry,
               reinterpret_cast<unsigned long *>(nextTask->context.sp));
  } else {
    if (old && nextTask) {
      // switch_context never returns, switches context to next task
      switch_context(old->stackPtr, nextTask->stackPtr);
    }
  }
}

namespace Tasks {

void yield() { taskManager.schedule(); }

} // namespace Tasks
