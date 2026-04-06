#include "order_book.h"

void OrderBook::AddOrder(uint64_t orderRef, uint32_t price, uint32_t quantity, char side) noexcept
{
  if ((side != 'B' && side != 'S') || mOrders.contains(orderRef))
  {
    ++mErrorCount;
    return;
  }

  const Side s = static_cast<Side>(side);
  mOrders[orderRef] = {price, quantity, s};

  if (s == Side::Bid)
  {
    mBids[price] += quantity;
  }
  else
  {
    mAsks[price] += quantity;
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

  ReduceLevel(iter->second, cancelledQuantity);
}

void OrderBook::ExecuteOrder(uint64_t orderRef, uint32_t executedQuantity) noexcept
{
  auto iter = mOrders.find(orderRef);
  if (iter == mOrders.end())
  {
    ++mErrorCount;
    return;
  }

  ReduceLevel(iter->second, executedQuantity);
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
    ReduceLevel(iter->second, iter->second.quantity);
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

void OrderBook::ReduceLevel(OrderEntry& entry, uint32_t quantity)
{
  auto& level = (entry.side == Side::Bid) ? mBids : mAsks;
  auto iter = level.find(entry.price);

  if (iter == level.end())
  {
    ++mErrorCount;
    return;
  }

  uint32_t& levelQuantity = iter->second;
  if (levelQuantity >= quantity && entry.quantity >= quantity)
  {
    levelQuantity -= quantity;
    entry.quantity -= quantity;
  }
  else if (levelQuantity >= entry.quantity)
  {
    // Cancel/execute quantity exceeds order's remaining quantity
    // Clamp to order's contribution
    levelQuantity -= entry.quantity;
    entry.quantity = 0;
    ++mErrorCount;
  }
  else
  {
    // Level qty < order qty book state corrupted
    // Clamp both to zero
    levelQuantity = 0;
    entry.quantity = 0;
    ++mErrorCount;
  }

  if (levelQuantity == 0)
  {
    level.erase(iter);
  }
}

std::optional<std::pair<uint32_t, uint32_t>> OrderBook::BestBid() const noexcept
{
  if (mBids.empty()) return std::nullopt;
  return *mBids.rbegin();
}

std::optional<std::pair<uint32_t, uint32_t>> OrderBook::BestAsk() const noexcept
{
  if (mAsks.empty()) return std::nullopt;
  return *mAsks.begin();
}

std::vector<std::pair<uint32_t, uint32_t>> OrderBook::BidDepth(size_t depth) const noexcept
{
  auto end = std::next(mBids.rbegin(), static_cast<ptrdiff_t>(std::min(depth, mBids.size())));
  return {mBids.rbegin(), end};
}

std::vector<std::pair<uint32_t, uint32_t>> OrderBook::AskDepth(size_t depth) const noexcept
{
  auto end = std::next(mAsks.begin(), static_cast<ptrdiff_t>(std::min(depth, mAsks.size())));
  return {mAsks.begin(), end};
}
