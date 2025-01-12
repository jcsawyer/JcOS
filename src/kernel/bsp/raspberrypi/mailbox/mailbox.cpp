#include "mailbox.hpp"
#include <arch/cpu.hpp>
#include <memory.h>
#include <print.hpp>
#include <stdint.h>
#include <time.hpp>

namespace Mailbox::RaspberryPi {

bool RaspberryPiMailbox::Call(Request *request, Response *response) {
  // Clear the data buffer
  // memset(Data, 0, sizeof(Data));
  // Determine the length of the request
  const uint32_t length = request->Data()[0] / 4;
  // Copy the request into the data buffer
  for (uint32_t i = 0; i < length; i++) {
    Data[i] = request->Data()[i];
  }

  const uint32_t mask = 0x0F;
  uintptr_t ptr = reinterpret_cast<uint64_t>(&Data);
  uintptr_t r = (ptr & ~mask) | (channel & 0xF);

  const auto timeout = Time::Duration::from_secs(2);
  auto end = Time::TimeManager::GetInstance()->uptime() + timeout;
  while (IsFull()) {
    if (Time::TimeManager::GetInstance()->uptime() > end) {
      warn("Failed to send message to mailbox within 2 second timeout...");
      return false;
    }
    CPU::nop();
  }

  *MAILBOX_WRITE = static_cast<uint32_t>(r);

  while (true) {
    end = Time::TimeManager::GetInstance()->uptime() + timeout;
    while (IsEmpty()) {
      if (Time::TimeManager::GetInstance()->uptime() > end) {
        warn("Failed to receive response from mailbox within 2 second "
             "timeout...");
        return false;
      }
      CPU::nop();
    }
    if (r == *MAILBOX_READ) {
      if (Data[1] == MAILBOX_RESPONSE) {
        const size_t responseSize = request->ResponseSize();
        uint32_t responseData[responseSize];
        for (uint32_t i = 0; i < responseSize; i++) {
          responseData[i] = Data[i + 5];
        }
        response->Data = responseData;
        return true;
      }
    }
  }
  return false;
}
} // namespace Mailbox::RaspberryPi