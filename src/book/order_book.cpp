#include "order_book.h"

#include <algorithm>

OrderBook::OrderBook()
{
  mOrders.reserve(128);
  mBids.reserve(64);
  mAsks.reserve(64);
}

void OrderBook::AddOrder(uint64_t orderRef, uint32_t price, uint32_t quantity, char side) noexcept
{
  if ((side != 'B' && side != 'S') || mOrders.contains(orderRef))
  {
    ++mErrorCount;
    return;
  }

  const Side s = static_cast<Side>(side);
  mOrders[orderRef] = {price, quantity, s};

  Order o = {orderRef, price, quantity};
  if (s == Side::Bid)
  {
    auto iter = std::upper_bound(mBids.begin(), mBids.end(), o);
    mBids.insert(iter, o);
  }
  else
  {
    auto iter = std::upper_bound(mAsks.begin(), mAsks.end(), o);
    mAsks.insert(iter, o);
  }
}

// CancelOrder and ExecuteOrder have identical book mutations
// They are kept as separate methods because they represent distinct ITCH 5.0 message types
// (OrderCancel vs OrderExecuted), and will diverge with more coverage
void OrderBook::CancelOrder(uint64_t orderRef, uint32_t cancelledQuantity) noexcept
{
  auto iter = mOrders.find(orderRef);
  if (iter == mOrders.end())
  {
    ++mErrorCount;
    return;
  }

  ReduceLevel(iter->first, iter->second, cancelledQuantity);
}

void OrderBook::ExecuteOrder(uint64_t orderRef, uint32_t executedQuantity) noexcept
{
  auto iter = mOrders.find(orderRef);
  if (iter == mOrders.end())
  {
    ++mErrorCount;
    return;
  }

  ReduceLevel(iter->first, iter->second, executedQuantity);
}

void OrderBook::DeleteOrder(uint64_t orderRef) noexcept
{
  auto iter = mOrders.find(orderRef);
  if (iter == mOrders.end())
  {
    ++mErrorCount;
    return;
  }

  if (iter->second.quantity > 0)
  {
    ReduceLevel(iter->first, iter->second, iter->second.quantity);
  }

  mOrders.erase(iter);
}

void OrderBook::ReplaceOrder(uint64_t oldOrderRef,
                             uint64_t newOrderRef,
                             uint32_t price,
                             uint32_t quantity) noexcept
{
  auto iter = mOrders.find(oldOrderRef);
  if (iter == mOrders.end())
  {
    ++mErrorCount;
    return;
  }

  char side = static_cast<char>(iter->second.side);
  DeleteOrder(oldOrderRef);
  AddOrder(newOrderRef, price, quantity, side);
}

bool OrderBook::ReduceLevel(uint64_t orderRef, OrderEntry& entry, uint32_t quantity) noexcept
{
  auto& book = (entry.side == Side::Bid) ? mBids : mAsks;

  const Order key{orderRef, entry.price, 0};
  auto lo = std::lower_bound(book.begin(), book.end(), key);
  auto hi = std::upper_bound(lo, book.end(), key);
  auto orderIter =
      std::find_if(lo, hi, [orderRef](const Order& o) { return o.orderRef == orderRef; });

  if (orderIter == hi)
  {
    ++mErrorCount;
    return false;
  }

  if (quantity <= entry.quantity)
  {
    orderIter->quantity -= quantity;
    entry.quantity -= quantity;
  }
  else
  {
    // Over-cancel: clamp to order's remaining quantity
    orderIter->quantity = 0;
    entry.quantity = 0;
    ++mErrorCount;
  }

  if (orderIter->quantity == 0)
  {
    book.erase(orderIter);
  }

  return true;
}

std::optional<Level> OrderBook::BestBid() const noexcept
{
  if (mBids.empty()) return std::nullopt;
  const uint32_t bestPrice = mBids.back().price;
  Level level{bestPrice, 0, 0};
  for (auto it = mBids.rbegin(); it != mBids.rend() && it->price == bestPrice; ++it)
  {
    ++level.orderCount;
    level.totalQty += it->quantity;
  }
  return level;
}

std::optional<Level> OrderBook::BestAsk() const noexcept
{
  if (mAsks.empty()) return std::nullopt;
  const uint32_t bestPrice = mAsks.front().price;
  Level level{bestPrice, 0, 0};
  for (auto it = mAsks.begin(); it != mAsks.end() && it->price == bestPrice; ++it)
  {
    ++level.orderCount;
    level.totalQty += it->quantity;
  }
  return level;
}

std::vector<Level> OrderBook::BidDepth(size_t depth) const noexcept
{
  std::vector<Level> result;
  result.reserve(depth);
  for (auto it = mBids.rbegin(); it != mBids.rend() && result.size() < depth;)
  {
    Level level{it->price, 0, 0};
    while (it != mBids.rend() && it->price == level.price)
    {
      ++level.orderCount;
      level.totalQty += it->quantity;
      ++it;
    }
    result.push_back(level);
  }
  return result;
}

std::vector<Level> OrderBook::AskDepth(size_t depth) const noexcept
{
  std::vector<Level> result;
  result.reserve(depth);
  for (auto it = mAsks.begin(); it != mAsks.end() && result.size() < depth;)
  {
    Level level{it->price, 0, 0};
    while (it != mAsks.end() && it->price == level.price)
    {
      ++level.orderCount;
      level.totalQty += it->quantity;
      ++it;
    }
    result.push_back(level);
  }
  return result;
}
