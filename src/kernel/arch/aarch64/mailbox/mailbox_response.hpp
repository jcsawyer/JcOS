#pragma once

#include <stdint.h>

namespace Mailbox {

class Response {
public:
  Response(uint32_t *data) : _data(data) {}
  virtual ~Response() = default;

protected:
  alignas(16) uint32_t *_data;
};

} // namespace Mailbox