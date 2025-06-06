#pragma once

#include <arch/aarch64/mailbox/mailbox_request.hpp>
#include <arch/aarch64/mailbox/mailbox_response.hpp>
#include <stddef.h>
#include <stdint.h>

namespace Mailbox::RaspberryPi::Messages {

class FirmwareVersionResponse : public Response {
public:
  FirmwareVersionResponse(uint32_t *data) : Response(data) {}
  uint32_t FirmwareRevision() { return Data[0]; }
};

class FirmwareVersionRequest : public Request {
public:
  FirmwareVersionRequest() : Request() {
    _data[0] = Request::MessageSize(6);
    _data[1] = Request::REQUEST_TAG;
    _data[2] = TAG;
    _data[3] = 4;
    _data[4] = 0;
    _data[5] = Request::END_TAG;
  }

  uint32_t *Data() override { return _data; }

  size_t ResponseSize() override { return 1; }

private:
  static const uint32_t TAG = 0x00000001;
  alignas(16) uint32_t *_data;
};
} // namespace Mailbox::RaspberryPi::Messages