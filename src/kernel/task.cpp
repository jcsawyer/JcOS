#include "task.hpp"
#include <print.hpp>

TaskManager taskManager;

extern "C" void switch_context(unsigned long *oldStackPtr,
                               unsigned long *newStackPtr);
extern "C" void enter_task(void (*entry)(void));

void TaskManager::init() {
  taskCount = 0;
  currentTask = -1;
}

void TaskManager::addTask(void (*entry)()) {
  info("ADD TASK: currentTask: %d, taskCount: %d, entry: %p", currentTask,
       taskCount, entry);

  if (taskCount >= MAX_TASKS)
    return;

  Task &t = tasks[taskCount];

  // Initialize context
  for (int i = 0; i < 12; ++i)
    (&t.context.x19)[i] = 0;

  t.context.x30 = reinterpret_cast<unsigned long>(entry); // LR = entry point

  // Stack pointer: top of stack, aligned 16 bytes
  unsigned long *stack_top = &t.stack[STACK_SIZE];
  t.context.sp = reinterpret_cast<unsigned long>(stack_top);

  // Point stackPtr to context for switch_context
  t.stackPtr = reinterpret_cast<unsigned long *>(&t.context);
  t.entry = entry;
  taskCount++;
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

  if (!nextTask->hasStarted) {
    nextTask->hasStarted = true;

    enter_task(nextTask->entry);
  } else {
    if (old && nextTask) {
      // switch_context never returns, switches context to next task
      switch_context(old->stackPtr, nextTask->stackPtr);
    }
  }
}
