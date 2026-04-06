#include <gtest/gtest.h>

#include "book/book_manager.h"
#include "book/order_book.h"

/*****************************************************************************
 * AddOrder
 *****************************************************************************/

TEST(OrderBook, BothSidesEmptyOnConstruction)
{
  OrderBook book;
  EXPECT_FALSE(book.BestBid().has_value());
  EXPECT_FALSE(book.BestAsk().has_value());
}

TEST(OrderBook, AddBid_BestBidReflectsIt)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  auto bid = book.BestBid();
  ASSERT_TRUE(bid.has_value());
  EXPECT_EQ(bid->first, 100'0000u);
  EXPECT_EQ(bid->second, 50u);
  EXPECT_FALSE(book.BestAsk().has_value());
}

TEST(OrderBook, AddAsk_BestAskReflectsIt)
{
  OrderBook book;
  book.AddOrder(2001, 105'0000, 30, 'S');
  auto ask = book.BestAsk();
  ASSERT_TRUE(ask.has_value());
  EXPECT_EQ(ask->first, 105'0000u);
  EXPECT_EQ(ask->second, 30u);
  EXPECT_FALSE(book.BestBid().has_value());
}

TEST(OrderBook, MultipleBidLevels_BestBidIsHighest)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.AddOrder(1002, 101'0000, 20, 'B');
  book.AddOrder(1003, 99'0000, 10, 'B');
  ASSERT_TRUE(book.BestBid().has_value());
  EXPECT_EQ(book.BestBid()->first, 101'0000u);
}

TEST(OrderBook, MultipleAsksLevels_BestAskIsLowest)
{
  OrderBook book;
  book.AddOrder(2001, 105'0000, 10, 'S');
  book.AddOrder(2002, 104'0000, 20, 'S');
  book.AddOrder(2003, 106'0000, 30, 'S');
  ASSERT_TRUE(book.BestAsk().has_value());
  EXPECT_EQ(book.BestAsk()->first, 104'0000u);
}

TEST(OrderBook, TwoOrdersSameLevel_QtyAggregated)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.AddOrder(1002, 100'0000, 30, 'B');
  ASSERT_TRUE(book.BestBid().has_value());
  EXPECT_EQ(book.BestBid()->second, 80u);
}

TEST(OrderBook, DuplicateOrderRef_ErrorCounted_BookUnchanged)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.AddOrder(1001, 101'0000, 20, 'B');
  EXPECT_EQ(book.ErrorCount(), 1u);
  auto bid = book.BestBid();
  ASSERT_TRUE(bid.has_value());
  EXPECT_EQ(bid->first,  100'0000u);
  EXPECT_EQ(bid->second, 50u);
}

TEST(OrderBook, InvalidSide_ErrorCounted_BookUnchanged)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'X');
  EXPECT_EQ(book.ErrorCount(), 1u);
  EXPECT_FALSE(book.BestBid().has_value());
  EXPECT_FALSE(book.BestAsk().has_value());
}

/*****************************************************************************
 * CancelOrder
 *****************************************************************************/

TEST(OrderBook, PartialCancel_LevelQtyReduced_OrderSurvives)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.CancelOrder(1001, 20);
  auto bid = book.BestBid();
  ASSERT_TRUE(bid.has_value());
  EXPECT_EQ(bid->second, 30u);
  EXPECT_EQ(book.ErrorCount(), 0u);
}

TEST(OrderBook, FullCancel_LevelErased)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.CancelOrder(1001, 50);
  EXPECT_FALSE(book.BestBid().has_value());
  EXPECT_EQ(book.ErrorCount(), 0u);
}

TEST(OrderBook, Cancel_DoesNotAffectOtherLevels)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.AddOrder(1002, 101'0000, 20, 'B');
  book.CancelOrder(1001, 50);
  auto bid = book.BestBid();
  ASSERT_TRUE(bid.has_value());
  EXPECT_EQ(bid->first, 101'0000u);
  EXPECT_EQ(bid->second, 20u);
}

TEST(OrderBook, Cancel_SharedPriceLevel_OnlyRemovesOrdersQty)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.AddOrder(1002, 100'0000, 30, 'B');
  book.CancelOrder(1001, 50);
  auto bid = book.BestBid();
  ASSERT_TRUE(bid.has_value());
  EXPECT_EQ(bid->second, 30u);
}

TEST(OrderBook, CancelUnknownRef_NoCrash_ErrorCounted)
{
  OrderBook book;
  book.CancelOrder(9999, 10);
  EXPECT_EQ(book.ErrorCount(), 1u);
}

TEST(OrderBook, OverCancel_ErrorCounted_LevelClamped)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.CancelOrder(1001, 100);
  EXPECT_FALSE(book.BestBid().has_value());
  EXPECT_EQ(book.ErrorCount(), 1u);
}

TEST(OrderBook, OverCancel_SharedLevel_OnlyRemovesOrderContribution)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.AddOrder(1002, 100'0000, 30, 'B');
  book.CancelOrder(1001, 200);
  auto bid = book.BestBid();
  ASSERT_TRUE(bid.has_value());
  EXPECT_EQ(bid->second, 30u);
  EXPECT_EQ(book.ErrorCount(), 1u);
}

TEST(OrderBook, CancelExceedsOrderQty_ButNotLevel_NoUnderflow)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.AddOrder(1002, 100'0000, 80, 'B');
  book.CancelOrder(1001, 70);  // 70 > order[1001].qty(50) but 70 <= level(130)
  EXPECT_EQ(book.ErrorCount(), 1u);
  auto bid = book.BestBid();
  ASSERT_TRUE(bid.has_value());
  EXPECT_EQ(bid->second, 80u);
}

/*****************************************************************************
 * ExecuteOrder
 *****************************************************************************/

TEST(OrderBook, PartialExecute_LevelQtyReduced)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.ExecuteOrder(1001, 20);
  auto bid = book.BestBid();
  ASSERT_TRUE(bid.has_value());
  EXPECT_EQ(bid->second, 30u);
  EXPECT_EQ(book.ErrorCount(), 0u);
}

TEST(OrderBook, FullExecute_LevelErased)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.ExecuteOrder(1001, 50);
  EXPECT_FALSE(book.BestBid().has_value());
  EXPECT_EQ(book.ErrorCount(), 0u);
}

TEST(OrderBook, ExecuteUnknownRef_NoCrash_ErrorCounted)
{
  OrderBook book;
  book.ExecuteOrder(9999, 10);
  EXPECT_EQ(book.ErrorCount(), 1u);
}

/*****************************************************************************
 * DeleteOrder
 *****************************************************************************/

TEST(OrderBook, Delete_OrderAndLevelGone)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.DeleteOrder(1001);
  EXPECT_FALSE(book.BestBid().has_value());
  EXPECT_EQ(book.ErrorCount(), 0u);
}

TEST(OrderBook, Delete_SharedLevel_OnlyAffectedOrderRemoved)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.AddOrder(1002, 100'0000, 30, 'B');
  book.DeleteOrder(1001);
  auto bid = book.BestBid();
  ASSERT_TRUE(bid.has_value());
  EXPECT_EQ(bid->second, 30u);
}

TEST(OrderBook, DeleteUnknownRef_NoCrash_ErrorCounted)
{
  OrderBook book;
  book.DeleteOrder(9999);
  EXPECT_EQ(book.ErrorCount(), 1u);
}

TEST(OrderBook, FullCancel_ThenDelete_NoCrash_NoError)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.CancelOrder(1001, 50);
  book.DeleteOrder(1001);
  EXPECT_FALSE(book.BestBid().has_value());
  EXPECT_EQ(book.ErrorCount(), 0u);
}

/*****************************************************************************
 * Error handling
 *****************************************************************************/

TEST(OrderBook, MultipleErrors_CountAccumulates)
{
  OrderBook book;
  book.CancelOrder(1, 1);
  book.ExecuteOrder(2, 1);
  book.DeleteOrder(3);
  EXPECT_EQ(book.ErrorCount(), 3u);
}

/*****************************************************************************
 * BidDepth / AskDepth
 *****************************************************************************/

TEST(OrderBook, BidDepth_DescendingOrder)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.AddOrder(1002, 101'0000, 20, 'B');
  book.AddOrder(1003, 99'0000, 10, 'B');
  auto depth = book.BidDepth(3);
  ASSERT_EQ(depth.size(), 3u);
  EXPECT_EQ(depth[0].first, 101'0000u);
  EXPECT_EQ(depth[1].first, 100'0000u);
  EXPECT_EQ(depth[2].first, 99'0000u);
}

TEST(OrderBook, BidDepth_LimitedToN)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.AddOrder(1002, 101'0000, 20, 'B');
  book.AddOrder(1003, 99'0000, 10, 'B');
  EXPECT_EQ(book.BidDepth(2).size(), 2u);
}

TEST(OrderBook, BidDepth_FewerLevelsThanN)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  EXPECT_EQ(book.BidDepth(5).size(), 1u);
}

/*****************************************************************************
 * ReplaceOrder
 *****************************************************************************/

TEST(OrderBook, Replace_NewRefVisibleAtNewPrice)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.ReplaceOrder(1001, 2001, 101'0000, 30);
  auto bid = book.BestBid();
  ASSERT_TRUE(bid.has_value());
  EXPECT_EQ(bid->first,  101'0000u);
  EXPECT_EQ(bid->second, 30u);
  EXPECT_EQ(book.ErrorCount(), 0u);
}

TEST(OrderBook, Replace_OldRefGone)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.ReplaceOrder(1001, 2001, 101'0000, 30);
  // old level must be gone
  auto depth = book.BidDepth(10);
  ASSERT_EQ(depth.size(), 1u);
  EXPECT_EQ(depth[0].first, 101'0000u);
}

TEST(OrderBook, Replace_PreservesSharedLevel)
{
  OrderBook book;
  book.AddOrder(1001, 100'0000, 50, 'B');
  book.AddOrder(1002, 100'0000, 20, 'B');
  book.ReplaceOrder(1001, 2001, 101'0000, 30);
  // 1002 still at 100'0000
  auto depth = book.BidDepth(10);
  ASSERT_EQ(depth.size(), 2u);
  EXPECT_EQ(depth[0].first,  101'0000u);
  EXPECT_EQ(depth[1].second, 20u);
}

TEST(OrderBook, Replace_SideInherited_AskStaysAsk)
{
  OrderBook book;
  book.AddOrder(2001, 105'0000, 30, 'S');
  book.ReplaceOrder(2001, 3001, 106'0000, 10);
  EXPECT_FALSE(book.BestBid().has_value());
  ASSERT_TRUE(book.BestAsk().has_value());
  EXPECT_EQ(book.BestAsk()->first, 106'0000u);
}

TEST(OrderBook, Replace_UnknownOldRef_ErrorCounted)
{
  OrderBook book;
  book.ReplaceOrder(9999, 1001, 100'0000, 50);
  EXPECT_EQ(book.ErrorCount(), 1u);
  EXPECT_FALSE(book.BestBid().has_value());
}

TEST(OrderBook, AskDepth_AscendingOrder)
{
  OrderBook book;
  book.AddOrder(2001, 105'0000, 30, 'S');
  book.AddOrder(2002, 104'0000, 40, 'S');
  book.AddOrder(2003, 106'0000, 10, 'S');
  auto depth = book.AskDepth(3);
  ASSERT_EQ(depth.size(), 3u);
  EXPECT_EQ(depth[0].first, 104'0000u);
  EXPECT_EQ(depth[1].first, 105'0000u);
  EXPECT_EQ(depth[2].first, 106'0000u);
}

/*****************************************************************************
 * BookManager
 *****************************************************************************/

TEST(BookManager, UnknownLocate_ReturnsNullptr)
{
  BookManager mgr;
  EXPECT_EQ(mgr.GetBook(42), nullptr);
}

TEST(BookManager, AddOrder_CreatesBook_BestBidVisible)
{
  BookManager mgr;
  mgr.AddOrder(1, 1001, 100'0000, 50, 'B');
  const OrderBook* book = mgr.GetBook(1);
  ASSERT_NE(book, nullptr);
  auto bid = book->BestBid();
  ASSERT_TRUE(bid.has_value());
  EXPECT_EQ(bid->first, 100'0000u);
}

TEST(BookManager, TwoLocates_IndependentBooks)
{
  BookManager mgr;
  mgr.AddOrder(1, 1001, 100'0000, 50, 'B');
  mgr.AddOrder(2, 2001, 200'0000, 30, 'S');
  ASSERT_NE(mgr.GetBook(1), nullptr);
  ASSERT_NE(mgr.GetBook(2), nullptr);
  EXPECT_FALSE(mgr.GetBook(1)->BestAsk().has_value());
  EXPECT_FALSE(mgr.GetBook(2)->BestBid().has_value());
}

TEST(BookManager, Cancel_RoutedToCorrectBook)
{
  BookManager mgr;
  mgr.AddOrder(1, 1001, 100'0000, 50, 'B');
  mgr.AddOrder(2, 2001, 200'0000, 30, 'B');
  mgr.CancelOrder(1, 1001, 50);
  EXPECT_FALSE(mgr.GetBook(1)->BestBid().has_value());
  ASSERT_TRUE(mgr.GetBook(2)->BestBid().has_value());
}

TEST(BookManager, Delete_RoutedToCorrectBook)
{
  BookManager mgr;
  mgr.AddOrder(1, 1001, 100'0000, 50, 'B');
  mgr.AddOrder(2, 2001, 200'0000, 30, 'B');
  mgr.DeleteOrder(1, 1001);
  EXPECT_FALSE(mgr.GetBook(1)->BestBid().has_value());
  ASSERT_TRUE(mgr.GetBook(2)->BestBid().has_value());
}

TEST(BookManager, Execute_RoutedToCorrectBook)
{
  BookManager mgr;
  mgr.AddOrder(1, 1001, 100'0000, 50, 'B');
  mgr.AddOrder(2, 2001, 200'0000, 30, 'B');
  mgr.ExecuteOrder(1, 1001, 50);
  EXPECT_FALSE(mgr.GetBook(1)->BestBid().has_value());
  ASSERT_TRUE(mgr.GetBook(2)->BestBid().has_value());
}

TEST(BookManager, Execute_UnknownLocate_NoCrash)
{
  BookManager mgr;
  mgr.ExecuteOrder(99, 1001, 10);
  EXPECT_EQ(mgr.GetBook(99), nullptr);
}

TEST(BookManager, Replace_RoutedToCorrectBook)
{
  BookManager mgr;
  mgr.AddOrder(1, 1001, 100'0000, 50, 'B');
  mgr.AddOrder(2, 2001, 200'0000, 30, 'B');
  mgr.ReplaceOrder(1, 1001, 3001, 101'0000, 25);
  const OrderBook* book1 = mgr.GetBook(1);
  ASSERT_NE(book1, nullptr);
  auto bid1 = book1->BestBid();
  ASSERT_TRUE(bid1.has_value());
  EXPECT_EQ(bid1->first,  101'0000u);
  EXPECT_EQ(bid1->second, 25u);
  const OrderBook* book2 = mgr.GetBook(2);
  ASSERT_NE(book2, nullptr);
  auto bid2 = book2->BestBid();
  ASSERT_TRUE(bid2.has_value());
  EXPECT_EQ(bid2->first, 200'0000u);
}

TEST(BookManager, Replace_UnknownLocate_NoCrash)
{
  BookManager mgr;
  mgr.ReplaceOrder(99, 1001, 2001, 100'0000, 50);
  EXPECT_EQ(mgr.GetBook(99), nullptr);
}
