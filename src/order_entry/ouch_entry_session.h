#pragma once

#include <cstdint>

#include "order_entry/isender.h"

enum class SessionState
{
  Disconnected,
  LoggedIn,
};

class OrderEntrySession
{
public:
  explicit OrderEntrySession(ISender& sender);

  void EnterOrder(char        side,
                  uint32_t    quantity,
                  const char* symbol,
                  uint64_t    price,
                  char        timeInForce,
                  char        display,
                  char        capacity,
                  char        interMarketSweepEligibility,
                  char        crossType,
                  const char* clOrdId);

  void CancelOrder(uint32_t userRefNum, uint32_t quantity);

  void SetLoggedIn() { mState = SessionState::LoggedIn; }

private:
  template <typename T>
  void SendMsg(const T& msg);

  static constexpr size_t kBufferSize = 256;

  ISender&     mSender;
  uint32_t     mNextUserRefNumber;
  SessionState mState;
  uint8_t      mBuffer[kBufferSize];
};
