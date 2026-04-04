#pragma once

#include <cstring>

#include "itch_message_types.h"

class ItchHandler
{
public:
  template <typename T>
  void Dispatch(const char* buffer)
  {
    T msg;
    memcpy(&msg, buffer, sizeof(T));
    OnMessage(msg);
  }

private:
  void OnMessage(const ItchAddOrder& msg);
  void OnMessage(const ItchOrderCancel& msg);
  void OnMessage(const ItchOrderExecuted& msg);
  void OnMessage(const ItchOrderDelete& msg);

  uint64_t mAddOrderCount = 0;
  uint64_t mOrderCancelCount = 0;
  uint64_t mOrderExecutedCount = 0;
  uint64_t mOrderDeleteCount = 0;
};
