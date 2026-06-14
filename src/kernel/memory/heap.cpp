#include "heap.hpp"

#include <backtrace.hpp>
#include <bsp/raspberrypi/memory.hpp>
#include <common.hpp>
#include <exceptions/asynchronous.hpp>
#include <memory.h>
#include <panic.hpp>
#include <print.hpp>
#include <synchronization.hpp>

namespace Memory {
namespace {
HeapAllocator gKernelHeapAllocator;

void printUsageLine(const char *label, size_t value) {
  if (value >= KiB) {
    size_t humanValue = value;
    const char *unit = "Byte";

    if (value >= GiB) {
      humanValue = div_ceil(value, GiB);
      unit = "GiB";
    } else if (value >= MiB) {
      humanValue = div_ceil(value, MiB);
      unit = "MiB";
    } else {
      humanValue = div_ceil(value, KiB);
      unit = "KiB";
    }

    info("      %s: %lu Byte (%lu %s)", label, value, humanValue, unit);
    return;
  }

  info("      %s: %lu Byte", label, value);
}

void debugPrintAllocation(const char *operation, void *ptr, size_t size) {
#ifdef DEBUG_PRINTS
  size_t start = reinterpret_cast<size_t>(ptr);
  debug("Kernel Heap: %s", operation);
  debug("      Size:     0x%lx (%lu Byte)", size, size);
  debug("      Start:    0x%016lX", start);
  debug("      End excl: 0x%016lX", start + size);
  Backtrace::print();
#else
  (void)operation;
  (void)ptr;
  (void)size;
#endif
}
} // namespace

size_t HeapAllocator::alignUp(size_t value, size_t alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

void HeapAllocator::init(size_t start, size_t size) {
  if (initialized) {
    panic("Kernel heap already initialized");
  }

  if (size < sizeof(FreeBlock)) {
    panic("Kernel heap too small");
  }

  heapStart = start;
  heapSize = size;
  usedBytes = 0;
  freeList = reinterpret_cast<FreeBlock *>(start);
  freeList->size = size;
  freeList->next = nullptr;
  initialized = true;
}

void *HeapAllocator::allocate(size_t size, size_t alignment) {
  if (!initialized) {
    panic("Kernel heap allocation before initialization");
  }

  if (size == 0) {
    size = 1;
  }

  if (alignment < alignof(void *)) {
    alignment = alignof(void *);
  }

  FreeBlock *previous = nullptr;
  FreeBlock *current = freeList;

  while (current != nullptr) {
    const size_t blockStart = reinterpret_cast<size_t>(current);
    const size_t blockEnd = blockStart + current->size;
    const size_t userStart =
        alignUp(blockStart + sizeof(AllocationHeader), alignment);
    const size_t headerStart = userStart - sizeof(AllocationHeader);
    const size_t totalSize = (userStart + size) - blockStart;

    if (blockStart + totalSize > blockEnd) {
      previous = current;
      current = current->next;
      continue;
    }

    const size_t remaining = blockEnd - (blockStart + totalSize);
    if (remaining > 0 && remaining < sizeof(FreeBlock)) {
      previous = current;
      current = current->next;
      continue;
    }

    if (previous == nullptr) {
      freeList = current->next;
    } else {
      previous->next = current->next;
    }

    if (remaining >= sizeof(FreeBlock)) {
      FreeBlock *tail = reinterpret_cast<FreeBlock *>(blockStart + totalSize);
      tail->size = remaining;
      tail->next = nullptr;
      insertFreeBlock(tail);
    }

    AllocationHeader *header =
        reinterpret_cast<AllocationHeader *>(headerStart);
    header->blockSize = totalSize;
    header->requestedSize = size;
    header->blockStart = blockStart;
    usedBytes += totalSize;

    void *result = reinterpret_cast<void *>(userStart);
    debugPrintAllocation("Allocation", result, size);
    return result;
  }

  panic("Kernel heap exhausted while allocating %lu bytes", size);
}

void HeapAllocator::insertFreeBlock(FreeBlock *block) {
  block->next = nullptr;

  if (freeList == nullptr ||
      reinterpret_cast<size_t>(block) < reinterpret_cast<size_t>(freeList)) {
    block->next = freeList;
    freeList = block;
    coalesce();
    return;
  }

  FreeBlock *current = freeList;
  while (current->next != nullptr && reinterpret_cast<size_t>(current->next) <
                                         reinterpret_cast<size_t>(block)) {
    current = current->next;
  }

  block->next = current->next;
  current->next = block;
  coalesce();
}

void HeapAllocator::coalesce() {
  FreeBlock *current = freeList;
  while (current != nullptr && current->next != nullptr) {
    const size_t currentEnd = reinterpret_cast<size_t>(current) + current->size;
    if (currentEnd == reinterpret_cast<size_t>(current->next)) {
      current->size += current->next->size;
      current->next = current->next->next;
      continue;
    }
    current = current->next;
  }
}

void HeapAllocator::deallocate(void *ptr) noexcept {
  if (ptr == nullptr) {
    return;
  }

  AllocationHeader *header = reinterpret_cast<AllocationHeader *>(
      reinterpret_cast<size_t>(ptr) - sizeof(AllocationHeader));

  if (header->blockStart < heapStart ||
      header->blockStart + header->blockSize > heapStart + heapSize) {
    panic("Kernel heap free outside heap bounds");
  }

  usedBytes -= header->blockSize;

  FreeBlock *block = reinterpret_cast<FreeBlock *>(header->blockStart);
  block->size = header->blockSize;
  insertFreeBlock(block);

  debugPrintAllocation("Free", ptr, header->requestedSize);
}

void HeapAllocator::printUsage() const {
  printUsageLine("Used", usedBytes);
  printUsageLine("Free", heapSize - usedBytes);
}

HeapAllocator &kernel_heap_allocator() { return gKernelHeapAllocator; }

void kernel_init_heap_allocator() {
  const size_t start = Memory::heapStart();
  const size_t size = Memory::heapEndExclusive() - Memory::heapStart();
  gKernelHeapAllocator.init(start, size);
}

void *kernel_heap_allocate(size_t size, size_t alignment) {
  void *result = nullptr;
  Exceptions::Asynchronous::execWithIrqMasked([&]() {
    result = gKernelHeapAllocator.allocate(size, alignment);
  });
  return result;
}

void kernel_heap_deallocate(void *ptr) noexcept {
  Exceptions::Asynchronous::execWithIrqMasked(
      [&]() { gKernelHeapAllocator.deallocate(ptr); });
}

} // namespace Memory

void *operator new(size_t size) {
  return Memory::kernel_heap_allocate(size, alignof(void *));
}

void *operator new[](size_t size) {
  return Memory::kernel_heap_allocate(size, alignof(void *));
}

void *operator new(size_t size, std::align_val_t alignment) {
  return Memory::kernel_heap_allocate(size, static_cast<size_t>(alignment));
}

void *operator new[](size_t size, std::align_val_t alignment) {
  return Memory::kernel_heap_allocate(size, static_cast<size_t>(alignment));
}

void operator delete(void *ptr) noexcept {
  Memory::kernel_heap_deallocate(ptr);
}

void operator delete[](void *ptr) noexcept {
  Memory::kernel_heap_deallocate(ptr);
}

void operator delete(void *ptr, size_t size) noexcept {
  (void)size;
  Memory::kernel_heap_deallocate(ptr);
}

void operator delete[](void *ptr, size_t size) noexcept {
  (void)size;
  Memory::kernel_heap_deallocate(ptr);
}

void operator delete(void *ptr, std::align_val_t alignment) noexcept {
  (void)alignment;
  Memory::kernel_heap_deallocate(ptr);
}

void operator delete[](void *ptr, std::align_val_t alignment) noexcept {
  (void)alignment;
  Memory::kernel_heap_deallocate(ptr);
}

void operator delete(void *ptr, size_t size,
                     std::align_val_t alignment) noexcept {
  (void)size;
  (void)alignment;
  Memory::kernel_heap_deallocate(ptr);
}

void operator delete[](void *ptr, size_t size,
                       std::align_val_t alignment) noexcept {
  (void)size;
  (void)alignment;
  Memory::kernel_heap_deallocate(ptr);
}
