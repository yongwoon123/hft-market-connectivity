#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

enum class Side : char { Bid = 'B', Ask = 'S' };

struct OrderEntry
{
  uint32_t price;
  uint32_t quantity;
  Side     side;
};

class OrderBook
{
public:
  void AddOrder(uint64_t orderRef, uint32_t price, uint32_t quantity, char side);
  void CancelOrder(uint64_t orderRef, uint32_t cancelQuantity);
  void ExecuteOrder(uint64_t orderRef, uint32_t executedQuantity);
  void DeleteOrder(uint64_t orderRef);

  uint64_t ErrorCount() const { return mErrorCount; }

  std::optional<std::pair<uint32_t, uint32_t>> BestBid() const;
  std::optional<std::pair<uint32_t, uint32_t>> BestAsk() const;

  std::vector<std::pair<uint32_t, uint32_t>> BidDepth(size_t depth) const;
  std::vector<std::pair<uint32_t, uint32_t>> AskDepth(size_t depth) const;

private:
  void ReduceLevel(OrderEntry& entry, uint32_t quantityReduction);

  std::map<uint32_t, uint32_t> mBids;  // Price -> Quantity
  std::map<uint32_t, uint32_t> mAsks;  // Price -> Quantity

  std::unordered_map<uint64_t, OrderEntry> mOrders;

  uint64_t mErrorCount = 0;
};
