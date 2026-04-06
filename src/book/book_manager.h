#pragma once

#include <cstdint>
#include <unordered_set>
#include <vector>

#include "book/order_book.h"

class BookManager
{
public:
  // All valid locate codes seen in 'R' messages, and the max locate value
  BookManager(const std::unordered_set<uint16_t>& validLocates, uint16_t maxLocate);

  void AddOrder(uint16_t locate, uint64_t orderRef, uint32_t price, uint32_t quantity, char side) noexcept;
  void CancelOrder(uint16_t locate, uint64_t orderRef, uint32_t cancelledQuantity) noexcept;
  void ExecuteOrder(uint16_t locate, uint64_t orderRef, uint32_t executedQuantity) noexcept;
  void DeleteOrder(uint16_t locate, uint64_t orderRef) noexcept;
  void ReplaceOrder(uint16_t locate, uint64_t oldOrderRef, uint64_t newOrderRef, uint32_t price, uint32_t quantity) noexcept;

  const OrderBook* GetBook(uint16_t locate) const noexcept;

  const std::unordered_set<uint16_t>& ValidLocates() const noexcept { return mValidLocates; }

private:
  std::vector<OrderBook>       mBooks;        // flat array indexed by stockLocate
  std::unordered_set<uint16_t> mValidLocates; // cold-path validity, populated from 'R' messages
};
