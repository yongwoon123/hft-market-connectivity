#pragma once

#include "itch_handler.h"

template <typename TStrategy>
ItchHandler<TStrategy>::ItchHandler(BookManager& bookManager, TStrategy& strategy)
    : mBookManager(bookManager)
    , mStrategy(strategy)
{
}

template <typename TStrategy>
void ItchHandler<TStrategy>::OnMessage(const ItchAddOrder& msg)
{
  mBookManager.AddOrder(msg.StockLocate(),
                        msg.OrderReferenceNumber(),
                        msg.Price(),
                        msg.Shares(),
                        msg.BuyOrSellIndicator());

  if (const OrderBook* book = mBookManager.GetBook(msg.StockLocate()))
    mStrategy.OnBookUpdate(msg.StockLocate(), *book);
}

template <typename TStrategy>
void ItchHandler<TStrategy>::OnMessage(const ItchAddOrderMpid& msg)
{
  mBookManager.AddOrder(msg.StockLocate(),
                        msg.OrderReferenceNumber(),
                        msg.Price(),
                        msg.Shares(),
                        msg.BuyOrSellIndicator());

  if (const OrderBook* book = mBookManager.GetBook(msg.StockLocate()))
    mStrategy.OnBookUpdate(msg.StockLocate(), *book);
}

template <typename TStrategy>
void ItchHandler<TStrategy>::OnMessage(const ItchOrderReplace& msg)
{
  mBookManager.ReplaceOrder(msg.StockLocate(),
                            msg.OriginalOrderReferenceNumber(),
                            msg.NewOrderReferenceNumber(),
                            msg.Price(),
                            msg.Shares());

  if (const OrderBook* book = mBookManager.GetBook(msg.StockLocate()))
    mStrategy.OnBookUpdate(msg.StockLocate(), *book);
}

template <typename TStrategy>
void ItchHandler<TStrategy>::OnMessage(const ItchOrderCancel& msg)
{
  mBookManager.CancelOrder(msg.StockLocate(), msg.OrderReferenceNumber(), msg.CancelledShares());

  if (const OrderBook* book = mBookManager.GetBook(msg.StockLocate()))
    mStrategy.OnBookUpdate(msg.StockLocate(), *book);
}

template <typename TStrategy>
void ItchHandler<TStrategy>::OnMessage(const ItchOrderExecuted& msg)
{
  mBookManager.ExecuteOrder(msg.StockLocate(), msg.OrderReferenceNumber(), msg.ExecutedShares());

  if (const OrderBook* book = mBookManager.GetBook(msg.StockLocate()))
    mStrategy.OnBookUpdate(msg.StockLocate(), *book);
}

template <typename TStrategy>
void ItchHandler<TStrategy>::OnMessage(const ItchOrderExecutedWithPrice& msg)
{
  mBookManager.ExecuteOrder(msg.StockLocate(), msg.OrderReferenceNumber(), msg.ExecutedShares());

  if (const OrderBook* book = mBookManager.GetBook(msg.StockLocate()))
    mStrategy.OnBookUpdate(msg.StockLocate(), *book);
}

template <typename TStrategy>
void ItchHandler<TStrategy>::OnMessage(const ItchOrderDelete& msg)
{
  mBookManager.DeleteOrder(msg.StockLocate(), msg.OrderReferenceNumber());

  if (const OrderBook* book = mBookManager.GetBook(msg.StockLocate()))
    mStrategy.OnBookUpdate(msg.StockLocate(), *book);
}
