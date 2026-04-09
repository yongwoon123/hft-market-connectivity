#include "strategy/passive_quoter.h"

#include <algorithm>

PassiveQuoter::PassiveQuoter(uint16_t locate, const char* symbol, OrderEntrySession& session)
    : mLocate(locate)
    , mSession(session)
    , mOrderSent(false)
{
  std::copy(symbol, symbol + 8, mSymbol.begin());
}

void PassiveQuoter::OnBookUpdate(uint16_t locate, const OrderBook& book)
{
  // TODO: filter by mLocate
  if (mLocate != locate)
  {
    return;
  }

  // TODO: check mOrderSent
  if (mOrderSent)
  {
    return;
  }

  // TODO: get BestBid and BestAsk, return if either missing
  std::optional bestBid = book.BestBid();
  std::optional bestAsk = book.BestAsk();
  if (!(bestBid && bestAsk))
  {
    return;
  }

  // TODO: return if locked/crossed market (bid >= ask)
  if (bestBid->price >= bestAsk->price)
  {
    return;
  }

  // TODO: call mSession.EnterOrder with the right params
  //   side: 'B'
  //   quantity: 100
  //   symbol: ???   <-- think about this one
  //   price: bestBid + 1
  //   timeInForce, display, capacity, interMarketSweepEligibility, crossType, clOrdId: pick
  //   sensible values
  mSession.EnterOrder('B',
                      100,
                      mSymbol.data(),
                      static_cast<uint64_t>(bestBid->price) + 1,
                      '0',
                      'Y',
                      'P',
                      'N',
                      'N',
                      "12345789012345");

  // TODO: set mOrderSent = true
  mOrderSent = true;
}
