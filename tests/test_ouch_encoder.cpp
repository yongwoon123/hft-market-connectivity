#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include "order_entry/isender.h"
#include "order_entry/ouch_encoder.h"
#include "order_entry/ouch_entry_session.h"
#include "order_entry/ouch_message_types.h"

/*****************************************************************************
 * BufferCaptureSender
 *****************************************************************************/

struct BufferCaptureSender : ISender
{
  void Send(const uint8_t* data, size_t len) override
  {
    captured.assign(data, data + len);
  }

  std::vector<uint8_t> captured;
};

/*****************************************************************************
 * Helpers
 *****************************************************************************/

static uint32_t read_u32_be(const uint8_t* p)
{
  return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) |
         (uint32_t(p[2]) << 8)  |  uint32_t(p[3]);
}

static uint64_t read_u64_be(const uint8_t* p)
{
  return (uint64_t(p[0]) << 56) | (uint64_t(p[1]) << 48) |
         (uint64_t(p[2]) << 40) | (uint64_t(p[3]) << 32) |
         (uint64_t(p[4]) << 24) | (uint64_t(p[5]) << 16) |
         (uint64_t(p[6]) <<  8) |  uint64_t(p[7]);
}

static uint16_t read_u16_be(const uint8_t* p)
{
  return (uint16_t(p[0]) << 8) | uint16_t(p[1]);
}

/*****************************************************************************
 * OuchEncoder - EnterOrder
 *****************************************************************************/

TEST(OuchEncoder, EnterOrder_TotalLength)
{
  OuchEnterOrder msg(1, 'B', 100, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
  uint8_t buf[256]{};
  size_t written = OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  EXPECT_EQ(written, 3 + 47);
}

TEST(OuchEncoder, EnterOrder_SoupBinTcpHeader)
{
  OuchEnterOrder msg(1, 'B', 100, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
  uint8_t buf[256]{};
  OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));

  // packet_length = 1 + 47 = 48
  EXPECT_EQ(read_u16_be(buf), 48);
  // packet_type = 'U'
  EXPECT_EQ(buf[2], 'U');
}

TEST(OuchEncoder, EnterOrder_MessageType)
{
  OuchEnterOrder msg(1, 'B', 100, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
  uint8_t buf[256]{};
  OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  // OUCH message type byte at buf[3]
  EXPECT_EQ(buf[3], 'O');
}

TEST(OuchEncoder, EnterOrder_UserRefNum)
{
  OuchEnterOrder msg(42, 'B', 100, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
  uint8_t buf[256]{};
  OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  // UserRefNum at offset 3+1=4, 4 bytes big-endian
  EXPECT_EQ(read_u32_be(buf + 4), 42u);
}

TEST(OuchEncoder, EnterOrder_Side)
{
  OuchEnterOrder msg(1, 'S', 100, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
  uint8_t buf[256]{};
  OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  // Side at offset 3+5=8
  EXPECT_EQ(buf[8], 'S');
}

TEST(OuchEncoder, EnterOrder_Quantity)
{
  OuchEnterOrder msg(1, 'B', 500, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
  uint8_t buf[256]{};
  OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  // Quantity at offset 3+6=9, 4 bytes big-endian
  EXPECT_EQ(read_u32_be(buf + 9), 500u);
}

TEST(OuchEncoder, EnterOrder_Symbol)
{
  OuchEnterOrder msg(1, 'B', 100, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
  uint8_t buf[256]{};
  OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  // Symbol at offset 3+10=13, 8 bytes
  EXPECT_EQ(memcmp(buf + 13, "AAPL    ", 8), 0);
}

TEST(OuchEncoder, EnterOrder_Price)
{
  OuchEnterOrder msg(1, 'B', 100, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
  uint8_t buf[256]{};
  OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  // Price at offset 3+18=21, 8 bytes big-endian
  EXPECT_EQ(read_u64_be(buf + 21), 1500000u);
}

TEST(OuchEncoder, EnterOrder_BufferTooSmall)
{
  OuchEnterOrder msg(1, 'B', 100, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
  uint8_t buf[10];
  size_t written = OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  EXPECT_EQ(written, 0u);
}

/*****************************************************************************
 * OuchEncoder - CancelOrder
 *****************************************************************************/

TEST(OuchEncoder, CancelOrder_TotalLength)
{
  OuchCancelOrder msg(1, 0);
  uint8_t buf[256]{};
  size_t written = OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  EXPECT_EQ(written, 3 + 11);
}

TEST(OuchEncoder, CancelOrder_SoupBinTcpHeader)
{
  OuchCancelOrder msg(1, 0);
  uint8_t buf[256]{};
  OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  // packet_length = 1 + 11 = 12
  EXPECT_EQ(read_u16_be(buf), 12);
  EXPECT_EQ(buf[2], 'U');
}

TEST(OuchEncoder, CancelOrder_MessageType)
{
  OuchCancelOrder msg(1, 0);
  uint8_t buf[256]{};
  OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  EXPECT_EQ(buf[3], 'X');
}

TEST(OuchEncoder, CancelOrder_UserRefNum)
{
  OuchCancelOrder msg(99, 0);
  uint8_t buf[256]{};
  OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  // UserRefNum at offset 3+1=4, 4 bytes big-endian
  EXPECT_EQ(read_u32_be(buf + 4), 99u);
}

TEST(OuchEncoder, CancelOrder_Quantity)
{
  OuchCancelOrder msg(1, 250);
  uint8_t buf[256]{};
  OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  // Quantity at offset 3+5=8, 4 bytes big-endian
  EXPECT_EQ(read_u32_be(buf + 8), 250u);
}

TEST(OuchEncoder, EnterOrder_ClOrdId)
{
  OuchEnterOrder msg(1, 'B', 100, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "myorder       ");
  uint8_t buf[256]{};
  OuchEncoder::Encode<OuchEncoder::kUnsequenced>(msg, buf, sizeof(buf));
  // ClOrdId at offset 3+31=34, 14 bytes
  EXPECT_EQ(memcmp(buf + 34, "myorder       ", 14), 0);
}

/*****************************************************************************
 * OrderEntrySession
 *****************************************************************************/

TEST(OrderEntrySession, UserRefNumIncrementsOnEachEnterOrder)
{
  BufferCaptureSender sender;
  OrderEntrySession   session(sender);
  session.SetLoggedIn();  // see note below

  session.EnterOrder('B', 100, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
  uint32_t first = read_u32_be(sender.captured.data() + 4);

  session.EnterOrder('B', 200, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
  uint32_t second = read_u32_be(sender.captured.data() + 4);

  EXPECT_LT(first, second);
}

TEST(OrderEntrySession, UserRefNumStrictlyIncreasing)
{
  BufferCaptureSender sender;
  OrderEntrySession   session(sender);
  session.SetLoggedIn();

  uint32_t prev = 0;
  for (int i = 0; i < 5; ++i)
  {
    session.EnterOrder('B', 100, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
    uint32_t curr = read_u32_be(sender.captured.data() + 4);
    EXPECT_GT(curr, prev);
    prev = curr;
  }
}

TEST(OrderEntrySession, DisconnectedSessionDoesNotSend)
{
  BufferCaptureSender sender;
  OrderEntrySession   session(sender);
  // state is Disconnected by default - no SetLoggedIn()

  session.EnterOrder('B', 100, "AAPL    ", 1500000, '0', 'Y', 'P', 'Y', 'N', "clordid       ");
  EXPECT_TRUE(sender.captured.empty());
}

TEST(OrderEntrySession, CancelOrderDisconnectedDoesNotSend)
{
  BufferCaptureSender sender;
  OrderEntrySession   session(sender);

  session.CancelOrder(77, 0);
  EXPECT_TRUE(sender.captured.empty());
}

TEST(OrderEntrySession, CancelOrderSendsCorrectUserRefNum)
{
  BufferCaptureSender sender;
  OrderEntrySession   session(sender);
  session.SetLoggedIn();

  session.CancelOrder(77, 0);
  EXPECT_EQ(read_u32_be(sender.captured.data() + 4), 77u);
}
