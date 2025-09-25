#include "syscall.hpp"
#include <console/console.hpp>
#include <print.hpp>
#include <syscall.hpp>

namespace Syscall {

uint64_t dispatch(uint64_t nr, uint64_t a0, uint64_t a1, uint64_t a2,
                  uint64_t a3) {
  switch (nr) {
  case WRITE: {
    const char *buf = reinterpret_cast<const char *>(a0);
    uint64_t len = a1;
    Console::Console *console = Console::Console::GetInstance();
    for (uint64_t i = 0; i < len; i++) {
      console->printChar(buf[i]);
    }
    return len;
  }
  case EXIT: {
    info("Process exit called");
    info("\tExited with error code %d", a0);
    while (1) {
      asm volatile("wfi");
    }
    return 0;
  }
  default:
    warn("Unknown syscall\n");
    return (uint64_t)-1;
  }
}

} // namespace Syscall
