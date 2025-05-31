#pragma once
#include <stdint.h>

#define MAX_TASKS 3
#define STACK_SIZE 1024

struct Context {
  unsigned long x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30;
  unsigned long sp;
};

struct Task {
  Context context;
  unsigned long *stackPtr = nullptr;
  unsigned long stack[STACK_SIZE];
  void (*entry)() = nullptr;
  bool hasStarted = false;
  const char *name = nullptr;
  int id = -1; // Unique identifier for the task
};

class TaskManager {
public:
  void init();
  void addTask(const char *name, void (*entryPoint)());
  void schedule();
  Task *current();
  int currentTask = -1;

private:
  Task tasks[MAX_TASKS];
  int taskCount = 0;
  int idCounter;
};

extern TaskManager taskManager;