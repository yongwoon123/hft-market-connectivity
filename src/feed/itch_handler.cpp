#include "itch_handler.h"

#include "book/book_manager.h"

ItchHandler::ItchHandler(BookManager& bookManager)
    : mBookManager(bookManager)
{
}

void ItchHandler::OnMessage(const ItchAddOrder& msg)
{
  mBookManager.AddOrder(msg.StockLocate(),
                        msg.OrderReferenceNumber(),
                        msg.Price(),
                        msg.Shares(),
                        msg.BuyOrSellIndicator());
}

void ItchHandler::OnMessage(const ItchAddOrderMpid& msg)
{
  mBookManager.AddOrder(msg.StockLocate(),
                        msg.OrderReferenceNumber(),
                        msg.Price(),
                        msg.Shares(),
                        msg.BuyOrSellIndicator());
}

void ItchHandler::OnMessage(const ItchOrderReplace& msg)
{
  mBookManager.ReplaceOrder(msg.StockLocate(),
                            msg.OriginalOrderReferenceNumber(),
                            msg.NewOrderReferenceNumber(),
                            msg.Price(),
                            msg.Shares());
}

void ItchHandler::OnMessage(const ItchOrderCancel& msg)
{
  mBookManager.CancelOrder(msg.StockLocate(), msg.OrderReferenceNumber(), msg.CancelledShares());
}

void ItchHandler::OnMessage(const ItchOrderExecuted& msg)
{
  mBookManager.ExecuteOrder(msg.StockLocate(), msg.OrderReferenceNumber(), msg.ExecutedShares());
}

void ItchHandler::OnMessage(const ItchOrderDelete& msg)
{
  mBookManager.DeleteOrder(msg.StockLocate(), msg.OrderReferenceNumber());
}
