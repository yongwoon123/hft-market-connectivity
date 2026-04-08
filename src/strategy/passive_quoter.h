#pragma once

#include <array>
#include <cstdint>

#include "book/order_book.h"
#include "order_entry/ouch_entry_session.h"

class PassiveQuoter
{
public:
  PassiveQuoter(uint16_t locate, const char* symbol, OrderEntrySession& session);

  void OnBookUpdate(uint16_t locate, const OrderBook& book);

private:
  const uint16_t mLocate;
  std::array<char, 8> mSymbol;
  OrderEntrySession& mSession;
  bool mOrderSent;
};
