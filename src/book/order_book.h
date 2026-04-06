#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

enum class Side : char
{
  Bid = 'B',
  Ask = 'S'
};

struct OrderEntry
{
  uint32_t price;
  uint32_t quantity;
  Side side;
};

class OrderBook
{
public:
  void AddOrder(uint64_t orderRef, uint32_t price, uint32_t quantity, char side) noexcept;
  void CancelOrder(uint64_t orderRef, uint32_t cancelledQuantity) noexcept;
  void ExecuteOrder(uint64_t orderRef, uint32_t executedQuantity) noexcept;
  void DeleteOrder(uint64_t orderRef) noexcept;
  void ReplaceOrder(uint64_t oldOrderRef, uint64_t newOrderRef, uint32_t price, uint32_t quantity) noexcept;

  uint64_t ErrorCount() const noexcept { return mErrorCount; }

  std::optional<std::pair<uint32_t, uint32_t>> BestBid() const noexcept;
  std::optional<std::pair<uint32_t, uint32_t>> BestAsk() const noexcept;

  std::vector<std::pair<uint32_t, uint32_t>> BidDepth(size_t depth) const noexcept;
  std::vector<std::pair<uint32_t, uint32_t>> AskDepth(size_t depth) const noexcept;

private:
  void ReduceLevel(OrderEntry& entry, uint32_t quantityReduction);

  std::map<uint32_t, uint32_t> mBids;  // Price -> Quantity
  std::map<uint32_t, uint32_t> mAsks;  // Price -> Quantity

  std::unordered_map<uint64_t, OrderEntry> mOrders;

  uint64_t mErrorCount = 0;
};
