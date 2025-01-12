#pragma once

#include <arch/aarch64/mailbox/mailbox_request.hpp>
#include <arch/aarch64/mailbox/mailbox_response.hpp>
#include <stddef.h>
#include <stdint.h>

namespace Mailbox::RaspberryPi::Messages {

class BoardRevisionResponse : public Response {
public:
  BoardRevisionResponse(uint32_t *data) : Response(data) {}
  uint32_t BoardRevision() { return Data[0]; }
};

class BoardRevisionRequest : public Request {
public:
  BoardRevisionRequest() : Request() {
    _data[0] = Request::MessageSize(5);
    _data[1] = Request::REQUEST_TAG;
    _data[2] = TAG;
    _data[3] = 8;
    _data[4] = Request::END_TAG;
  }

  uint32_t *Data() override { return _data; }

  size_t ResponseSize() override { return 1; }

private:
  static const uint32_t TAG = 0x00010002;
  alignas(16) uint32_t *_data;
};
} // namespace Mailbox::RaspberryPi::Messages