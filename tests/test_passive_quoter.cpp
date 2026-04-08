#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <unordered_set>
#include <vector>

#include "book/book_manager.h"
#include "feed/itch_handler.h"
#include "feed/itch_message_types.h"
#include "order_entry/isender.h"
#include "order_entry/ouch_entry_session.h"
#include "order_entry/ouch_message_types.h"
#include "strategy/passive_quoter.h"

/*****************************************************************************
 * Helpers
 *****************************************************************************/

struct CallCountingSender : ISender
{
  void Send(const uint8_t* data, size_t len) override
  {
    captured.assign(data, data + len);
    callCount++;
  }

  std::vector<uint8_t> captured;
  int                  callCount = 0;
};

static BookManager makeManager(std::initializer_list<uint16_t> locates)
{
  std::unordered_set<uint16_t> set(locates);
  uint16_t maxLocate = set.empty() ? 0 : *std::max_element(set.begin(), set.end());
  return BookManager(set, maxLocate);
}

// Reads a decoded OuchEnterOrder from captured bytes (skipping 3-byte SoupBinTCP header)
static const OuchEnterOrder* decodeEnterOrder(const std::vector<uint8_t>& buf)
{
  return reinterpret_cast<const OuchEnterOrder*>(buf.data() + 3);
}

/*****************************************************************************
 * PassiveQuoter - unit tests (OnBookUpdate called directly)
 *****************************************************************************/

TEST(PassiveQuoter, SendsOrderOnFirstTwoSidedBook)
{
  BookManager       bm  = makeManager({1});
  CallCountingSender sender;
  OrderEntrySession session{sender};
  session.SetLoggedIn();
  PassiveQuoter     quoter{1, "A       ", session};

  bm.AddOrder(1, 1001, 100'0000, 200, 'B');  // bid at $10.00
  bm.AddOrder(1, 1002, 101'0000, 100, 'S');  // ask at $10.10

  const OrderBook* book = bm.GetBook(1);
  ASSERT_NE(book, nullptr);
  quoter.OnBookUpdate(1, *book);

  ASSERT_EQ(sender.callCount, 1);
  ASSERT_EQ(sender.captured.size(), 50u);  // 3 SoupBinTCP + 47 OUCH EnterOrder

  const OuchEnterOrder* msg = decodeEnterOrder(sender.captured);
  EXPECT_EQ(msg->GetType(),     'O');
  EXPECT_EQ(msg->GetSide(),     'B');
  EXPECT_EQ(msg->GetQuantity(), 100u);
  EXPECT_EQ(msg->GetPrice(),    static_cast<uint64_t>(100'0000) + 1);  // bestBid + 1 tick
}

TEST(PassiveQuoter, SendsOnce_SecondUpdateDoesNotSendAgain)
{
  BookManager       bm  = makeManager({1});
  CallCountingSender sender;
  OrderEntrySession session{sender};
  session.SetLoggedIn();
  PassiveQuoter     quoter{1, "A       ", session};

  bm.AddOrder(1, 1001, 100'0000, 200, 'B');
  bm.AddOrder(1, 1002, 101'0000, 100, 'S');

  const OrderBook* book = bm.GetBook(1);
  ASSERT_NE(book, nullptr);
  quoter.OnBookUpdate(1, *book);
  quoter.OnBookUpdate(1, *book);

  EXPECT_EQ(sender.callCount, 1);
}

TEST(PassiveQuoter, DoesNotSendOnOneSidedBook)
{
  BookManager       bm  = makeManager({1});
  CallCountingSender sender;
  OrderEntrySession session{sender};
  session.SetLoggedIn();
  PassiveQuoter     quoter{1, "A       ", session};

  bm.AddOrder(1, 1001, 100'0000, 200, 'B');  // bid only, no ask

  const OrderBook* book = bm.GetBook(1);
  ASSERT_NE(book, nullptr);
  quoter.OnBookUpdate(1, *book);

  EXPECT_EQ(sender.callCount, 0);
}

TEST(PassiveQuoter, DoesNotSendOnWrongLocate)
{
  BookManager       bm  = makeManager({1, 2});
  CallCountingSender sender;
  OrderEntrySession session{sender};
  session.SetLoggedIn();
  PassiveQuoter     quoter{2, "B       ", session};  // watching locate 2

  bm.AddOrder(1, 1001, 100'0000, 200, 'B');
  bm.AddOrder(1, 1002, 101'0000, 100, 'S');

  const OrderBook* book = bm.GetBook(1);
  ASSERT_NE(book, nullptr);
  quoter.OnBookUpdate(1, *book);  // update for locate 1, not 2

  EXPECT_EQ(sender.callCount, 0);
}

TEST(PassiveQuoter, DoesNotSendOnLockedMarket)
{
  BookManager       bm  = makeManager({1});
  CallCountingSender sender;
  OrderEntrySession session{sender};
  session.SetLoggedIn();
  PassiveQuoter     quoter{1, "A       ", session};

  bm.AddOrder(1, 1001, 100'0000, 200, 'B');
  bm.AddOrder(1, 1002, 100'0000, 100, 'S');  // ask == bid: locked market

  const OrderBook* book = bm.GetBook(1);
  ASSERT_NE(book, nullptr);
  quoter.OnBookUpdate(1, *book);

  EXPECT_EQ(sender.callCount, 0);
}

/*****************************************************************************
 * Integration test - ItchHandler<PassiveQuoter> wiring
 *****************************************************************************/

// Builds a raw ItchAddOrder buffer with the given fields (big-endian)
static std::vector<uint8_t> makeAddOrderBytes(uint16_t locate,
                                              uint64_t orderRef,
                                              char     side,
                                              uint32_t shares,
                                              uint32_t price)
{
  std::vector<uint8_t> buf(36, 0);
  buf[0] = 'A';
  buf[1] = static_cast<uint8_t>(locate >> 8);
  buf[2] = static_cast<uint8_t>(locate);
  // trackingNumber [3-4]: zero
  // timestamp [5-10]: zero
  // orderRef [11-18]: big-endian uint64
  for (int i = 0; i < 8; ++i)
    buf[11 + i] = static_cast<uint8_t>(orderRef >> (56 - 8 * i));
  buf[19] = static_cast<uint8_t>(side);
  // shares [20-23]: big-endian uint32
  buf[20] = static_cast<uint8_t>(shares >> 24);
  buf[21] = static_cast<uint8_t>(shares >> 16);
  buf[22] = static_cast<uint8_t>(shares >> 8);
  buf[23] = static_cast<uint8_t>(shares);
  // stockName [24-31]: spaces
  std::fill(buf.begin() + 24, buf.begin() + 32, ' ');
  // price [32-35]: big-endian uint32
  buf[32] = static_cast<uint8_t>(price >> 24);
  buf[33] = static_cast<uint8_t>(price >> 16);
  buf[34] = static_cast<uint8_t>(price >> 8);
  buf[35] = static_cast<uint8_t>(price);
  return buf;
}

TEST(PassiveQuoterIntegration, ItchHandlerCallsStrategyAfterBookMutation)
{
  BookManager       bm  = makeManager({1});
  CallCountingSender sender;
  OrderEntrySession session{sender};
  session.SetLoggedIn();
  PassiveQuoter               quoter{1, "A       ", session};
  ItchHandler<PassiveQuoter>  handler{bm, quoter};

  // Dispatch a bid then an ask - second dispatch should trigger the strategy
  auto bidBytes = makeAddOrderBytes(1, 1001, 'B', 200, 100'0000);
  auto askBytes = makeAddOrderBytes(1, 1002, 'S', 100, 101'0000);

  handler.Dispatch<ItchAddOrder>(reinterpret_cast<const char*>(bidBytes.data()));
  EXPECT_EQ(sender.callCount, 0);  // one-sided - no order yet

  handler.Dispatch<ItchAddOrder>(reinterpret_cast<const char*>(askBytes.data()));
  EXPECT_EQ(sender.callCount, 1);  // two-sided - order sent

  const OuchEnterOrder* msg = decodeEnterOrder(sender.captured);
  EXPECT_EQ(msg->GetSide(),     'B');
  EXPECT_EQ(msg->GetQuantity(), 100u);
  EXPECT_EQ(msg->GetPrice(),    static_cast<uint64_t>(100'0000) + 1);
}
