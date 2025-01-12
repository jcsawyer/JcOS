#pragma once

#include "mailbox_response.hpp"
#include <stddef.h>
#include <stdint.h>

namespace Mailbox {
class Request {
public:
  Request() = default;
  ~Request() = default;
  virtual uint32_t *Data() = 0;
  virtual size_t ResponseSize() = 0;

protected:
  static const uint32_t TAG;
  static const uint32_t REQUEST_TAG = 0;
  static const uint32_t END_TAG = 0;
  uint32_t MessageSize(uint32_t size) { return size * 4; }
  uint32_t ResponseSize(uint32_t size) { return size / 4; }
  alignas(16) uint32_t *_data;
};

} // namespace Mailbox
