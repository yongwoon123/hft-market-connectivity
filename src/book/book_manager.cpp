#include "book_manager.h"

BookManager::BookManager(const std::unordered_set<uint16_t>& validLocates, uint16_t maxLocate)
    : mBooks(maxLocate + 1)
    , mValidLocates(validLocates)
{
}

void BookManager::AddOrder(uint16_t locate,
                           uint64_t orderRef,
                           uint32_t price,
                           uint32_t quantity,
                           char side) noexcept
{
  if (locate < mBooks.size())
  {
    mBooks[locate].AddOrder(orderRef, price, quantity, side);
  }
}

void BookManager::CancelOrder(uint16_t locate,
                              uint64_t orderRef,
                              uint32_t cancelledQuantity) noexcept
{
  if (locate < mBooks.size())
  {
    mBooks[locate].CancelOrder(orderRef, cancelledQuantity);
  }
}

void BookManager::ExecuteOrder(uint16_t locate,
                               uint64_t orderRef,
                               uint32_t executedQuantity) noexcept
{
  if (locate < mBooks.size())
  {
    mBooks[locate].ExecuteOrder(orderRef, executedQuantity);
  }
}

void BookManager::DeleteOrder(uint16_t locate, uint64_t orderRef) noexcept
{
  if (locate < mBooks.size())
  {
    mBooks[locate].DeleteOrder(orderRef);
  }
}

void BookManager::ReplaceOrder(uint16_t locate,
                               uint64_t oldOrderRef,
                               uint64_t newOrderRef,
                               uint32_t price,
                               uint32_t quantity) noexcept
{
  if (locate < mBooks.size())
  {
    mBooks[locate].ReplaceOrder(oldOrderRef, newOrderRef, price, quantity);
  }
}

const OrderBook* BookManager::GetBook(uint16_t locate) const noexcept
{
  if (!mValidLocates.contains(locate))
  {
    return nullptr;
  }
  return &mBooks[locate];
}
