#pragma once

#include <stdint.h>

namespace Mailbox {

class Response {
public:
  Response(uint32_t *data) : Data(data) {}
  virtual ~Response() = default;
  alignas(16) uint32_t *Data;
};

} // namespace Mailbox