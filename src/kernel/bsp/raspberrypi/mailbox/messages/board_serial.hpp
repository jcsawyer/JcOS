#pragma once

#include <arch/aarch64/mailbox/mailbox_request.hpp>
#include <arch/aarch64/mailbox/mailbox_response.hpp>
#include <stddef.h>
#include <stdint.h>

namespace Mailbox::RaspberryPi::Messages {

struct BoardSerial {
  uint32_t Low;
  uint32_t High;
};

class BoardSerialResponse : public Response {
public:
  BoardSerialResponse(uint32_t *data) : Response(data) {}
  BoardSerial BoardSerial() { return {Data[0], Data[1]}; }
};

class BoardSerialRequest : public Request {
public:
  BoardSerialRequest() : Request() {
    _data[0] = Request::MessageSize(8);
    _data[1] = Request::REQUEST_TAG;
    _data[2] = TAG;
    _data[3] = 8;
    _data[4] = 8;
    _data[5] = 0;
    _data[6] = 0;
    _data[7] = Request::END_TAG;
  }

  uint32_t *Data() override { return _data; }

  size_t ResponseSize() override { return 2; }

private:
  static const uint32_t TAG = 0x10004;
  alignas(16) uint32_t *_data;
};
} // namespace Mailbox::RaspberryPi::Messages