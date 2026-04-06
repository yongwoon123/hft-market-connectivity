#pragma once

#include <cstdint>
#include <unordered_map>

#include "book/order_book.h"

class BookManager
{
public:
  void AddOrder(uint16_t locate, uint64_t orderRef, uint32_t price, uint32_t quantity, char side) noexcept;
  void CancelOrder(uint16_t locate, uint64_t orderRef, uint32_t cancelQuantity) noexcept;
  void ExecuteOrder(uint16_t locate, uint64_t orderRef, uint32_t executedQuantity) noexcept;
  void DeleteOrder(uint16_t locate, uint64_t orderRef) noexcept;

  const OrderBook* GetBook(uint16_t locate) const noexcept;

private:
  std::unordered_map<uint16_t, OrderBook> mBooks;
};
