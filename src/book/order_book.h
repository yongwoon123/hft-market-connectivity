#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

enum class Side : char
{
  Bid = 'B',
  Ask = 'S'
};

struct Order
{
  uint64_t orderRef;
  uint32_t price;
  uint32_t quantity;

  bool operator<(const Order& rhs) const { return price < rhs.price; }
};

struct OrderEntry
{
  uint32_t price;
  uint32_t quantity;
  Side side;
};

struct Level
{
  uint32_t price;
  uint32_t orderCount;
  uint64_t totalQty;
};

class OrderBook
{
public:
  OrderBook();

  void AddOrder(uint64_t orderRef, uint32_t price, uint32_t quantity, char side) noexcept;
  void CancelOrder(uint64_t orderRef, uint32_t cancelledQuantity) noexcept;
  void ExecuteOrder(uint64_t orderRef, uint32_t executedQuantity) noexcept;
  void DeleteOrder(uint64_t orderRef) noexcept;
  void ReplaceOrder(uint64_t oldOrderRef, uint64_t newOrderRef, uint32_t price, uint32_t quantity) noexcept;

  uint64_t ErrorCount() const noexcept { return mErrorCount; }

  std::optional<Level> BestBid() const noexcept;
  std::optional<Level> BestAsk() const noexcept;

  std::vector<Level> BidDepth(size_t depth) const noexcept;
  std::vector<Level> AskDepth(size_t depth) const noexcept;

private:
  bool ReduceLevel(uint64_t orderRef, OrderEntry& entry, uint32_t quantityReduction) noexcept;

  std::vector<Order> mBids;  // Sorted Ascending back is best Bid
  std::vector<Order> mAsks;  // Sorted Ascending front is best Ask

  std::unordered_map<uint64_t, OrderEntry> mOrders;

  uint64_t mErrorCount = 0;
};
