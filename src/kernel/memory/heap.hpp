#pragma once

#include <stddef.h>

namespace Memory {

struct HeapUsage {
  size_t usedBytes;
  size_t totalBytes;
};

class HeapAllocator {
public:
  void init(size_t start, size_t size);
  void *allocate(size_t size, size_t alignment);
  void deallocate(void *ptr) noexcept;
  void printUsage() const;
  bool isInitialized() const { return initialized; }
  size_t usedBytes() const { return usedBytes_; }
  size_t totalBytes() const { return heapSize; }

private:
  struct FreeBlock {
    size_t size;
    FreeBlock *next;
  };

  struct AllocationHeader {
    size_t blockSize;
    size_t requestedSize;
    size_t blockStart;
  };

  static size_t alignUp(size_t value, size_t alignment);

  void insertFreeBlock(FreeBlock *block);
  void coalesce();

  FreeBlock *freeList = nullptr;
  size_t heapStart = 0;
  size_t heapSize = 0;
  size_t usedBytes_ = 0;
  bool initialized = false;
};

HeapAllocator &kernel_heap_allocator();
void kernel_init_heap_allocator();
HeapUsage kernel_heap_usage();
void *kernel_heap_allocate(size_t size, size_t alignment);
void kernel_heap_deallocate(void *ptr) noexcept;

} // namespace Memory
