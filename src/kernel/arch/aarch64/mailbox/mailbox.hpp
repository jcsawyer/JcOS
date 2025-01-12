#pragma once
#include "mailbox_request.hpp"
#include "mailbox_response.hpp"
#include <stdint.h>

namespace Mailbox {

class Mailbox {
public:
  virtual Response Call(Request *request) = 0;
  virtual bool IsFull() = 0;
  virtual bool IsEmpty() = 0;

protected:
  Mailbox() = default;
  ~Mailbox() = default;
  uint32_t MAILBOX_BASE;
  uint32_t MAIL_FULL;
  uint32_t MAIL_EMPTY;
};

} // namespace Mailbox