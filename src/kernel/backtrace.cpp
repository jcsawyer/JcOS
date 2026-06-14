#include <backtrace.hpp>

#include <arch/aarch64/backtrace.hpp>
#include <print.hpp>
#include <stdio/printf.h>
#include <symbols.hpp>

namespace Backtrace {
namespace {

struct PrintContext {
  size_t visibleFrameCount;
};

constexpr size_t INTERNAL_FRAMES_TO_SKIP = 2;

bool printFrame(const BacktraceItem &item, size_t frameIndex, void *context) {
  PrintContext *printContext = reinterpret_cast<PrintContext *>(context);

  if (frameIndex < INTERNAL_FRAMES_TO_SKIP) {
    return true;
  }

  const size_t visibleIndex = ++printContext->visibleFrameCount;

  switch (item.type) {
  case ItemType::InvalidFramePointer:
    printf_("      %2lu. ERROR! Encountered invalid frame pointer "
            "(0x%016lX) during backtrace\n",
            visibleIndex, static_cast<unsigned long>(item.address));
    return false;
  case ItemType::InvalidLink:
    printf_("      %2lu. ERROR! Link address (0x%016lX) is not contained in "
            "kernel .text section\n",
            visibleIndex, static_cast<unsigned long>(item.address));
    return false;
  case ItemType::Link: {
    const Symbols::KernelSymbol *symbol = Symbols::lookupSymbol(item.address);
    printf_("      %2lu. 0x%016lX | %s\n", visibleIndex,
            static_cast<unsigned long>(item.address),
            symbol ? symbol->name : "Symbol not found");
    return true;
  }
  }

  return false;
}

} // namespace

void print() {
  printf_("Backtrace:\n");
  printf_("      ------------------------------------------------------------"
          "------------------\n");
  printf_("          Address            Function containing address\n");
  printf_("      ------------------------------------------------------------"
          "------------------\n");

  PrintContext context{0};
  Arch::walk(printFrame, &context);

  if (context.visibleFrameCount == 0) {
    printf_("      ERROR! No valid stack frame found\n");
  }

  printf_("      ------------------------------------------------------------"
          "------------------\n");
}

} // namespace Backtrace
