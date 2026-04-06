#include "book_manager.h"

void BookManager::AddOrder(uint16_t locate,
                           uint64_t orderRef,
                           uint32_t price,
                           uint32_t quantity,
                           char side) noexcept
{
  mBooks[locate].AddOrder(orderRef, price, quantity, side);
}

void BookManager::CancelOrder(uint16_t locate, uint64_t orderRef, uint32_t cancelledQuantity) noexcept
{
  auto iter = mBooks.find(locate);
  if (iter == mBooks.end())
  {
    return;
  }

  iter->second.CancelOrder(orderRef, cancelledQuantity);
}

void BookManager::ExecuteOrder(uint16_t locate,
                               uint64_t orderRef,
                               uint32_t executedQuantity) noexcept
{
  auto iter = mBooks.find(locate);
  if (iter == mBooks.end())
  {
    return;
  }

  iter->second.ExecuteOrder(orderRef, executedQuantity);
}

void BookManager::DeleteOrder(uint16_t locate, uint64_t orderRef) noexcept
{
  auto iter = mBooks.find(locate);
  if (iter == mBooks.end())
  {
    return;
  }

  iter->second.DeleteOrder(orderRef);
}

void BookManager::ReplaceOrder(uint16_t locate,
                               uint64_t oldOrderRef,
                               uint64_t newOrderRef,
                               uint32_t price,
                               uint32_t quantity) noexcept
{
  auto iter = mBooks.find(locate);
  if (iter == mBooks.end())
  {
    return;
  }

  iter->second.ReplaceOrder(oldOrderRef, newOrderRef, price, quantity);
}

const OrderBook* BookManager::GetBook(uint16_t locate) const noexcept
{
  auto iter = mBooks.find(locate);
  if (iter == mBooks.end())
  {
    return nullptr;
  }

  return &iter->second;
}
