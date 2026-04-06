#include "ouch_entry_session.h"

#include "infra/clock.h"
#include "order_entry/ouch_encoder.h"
#include "order_entry/ouch_message_types.h"

OrderEntrySession::OrderEntrySession(ISender& sender)
    : mSender(sender)
    , mNextUserRefNumber(infra::nowSinceMidnight_ms())
    , mState(SessionState::Disconnected)
{
}

template <typename T>
void OrderEntrySession::SendMsg(const T& msg)
{
  const size_t len = OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, mBuffer, kBufferSize);
  if (len > 0)
    mSender.Send(mBuffer, len);
}

void OrderEntrySession::EnterOrder(char        side,
                                   uint32_t    quantity,
                                   const char* symbol,
                                   uint64_t    price,
                                   char        timeInForce,
                                   char        display,
                                   char        capacity,
                                   char        interMarketSweepEligibility,
                                   char        crossType,
                                   const char* clOrdId)
{
  if (mState == SessionState::Disconnected)
    return;

  SendMsg(OuchEnterOrder(mNextUserRefNumber++, side, quantity, symbol, price,
                         timeInForce, display, capacity, interMarketSweepEligibility,
                         crossType, clOrdId));
}

void OrderEntrySession::CancelOrder(uint32_t userRefNum, uint32_t quantity)
{
  if (mState == SessionState::Disconnected)
    return;

  SendMsg(OuchCancelOrder(userRefNum, quantity));
}
