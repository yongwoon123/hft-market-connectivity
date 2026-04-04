#include "itch_handler.h"

#include <iostream>

uint64_t GetTimespan(const std::span<const uint8_t, 6>& timestamp)
{
  uint64_t ts = 0;
  for (int i = 0; i < 6; ++i) ts = (ts << 8) | timestamp[i];
  return ts;
}

void ItchHandler::OnMessage(const ItchAddOrder& msg)
{
  if (mAddOrderCount < 20)
  {
    std::cout << "ts=" << std::to_string(GetTimespan(msg.Timestamp()))
              << " side=" << msg.BuyOrSellIndicator() << " stock=" << std::string(msg.StockName(), 8)
              << " price=" << msg.Price() / 10000 << "\n";
    ++mAddOrderCount;
  }
}

void ItchHandler::OnMessage(const ItchOrderCancel& msg)
{
  (void)msg;
  ++mOrderCancelCount;
}

void ItchHandler::OnMessage(const ItchOrderExecuted& msg)
{
  (void)msg;
  ++mOrderExecutedCount;
}

void ItchHandler::OnMessage(const ItchOrderDelete& msg)
{
  (void)msg;
  ++mOrderDeleteCount;
}
