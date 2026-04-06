#pragma once

#include <cstring>

#include "book/book_manager.h"
#include "itch_message_types.h"

class ItchHandler
{
public:
  ItchHandler(BookManager& bookManager);

  template <typename T>
  void Dispatch(const char* buffer)
  {
    T msg;
    memcpy(&msg, buffer, sizeof(T));
    OnMessage(msg);
  }

private:
  void OnMessage(const ItchAddOrder& msg);
  void OnMessage(const ItchAddOrderMpid& msg);
  void OnMessage(const ItchOrderReplace& msg);
  void OnMessage(const ItchOrderCancel& msg);
  void OnMessage(const ItchOrderExecuted& msg);
  void OnMessage(const ItchOrderExecutedWithPrice& msg);
  void OnMessage(const ItchOrderDelete& msg);

  BookManager& mBookManager;
};
