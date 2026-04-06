#pragma once

#include <cstdint>
#include <span>

// Fields are intentionally duplicated across structs rather than extracted into a common base.
// Inheritance breaks standard-layout, making offsetof on derived members undefined behaviour
// and preventing reliable static_assert layout validation against the wire format.

// 'S' — System Event
#pragma pack(push, 1)
struct ItchSystemEvent
{
public:
  uint16_t                    StockLocate()    const { return __builtin_bswap16(stockLocate); }
  uint16_t                    TrackingNumber() const { return __builtin_bswap16(trackingNumber); }
  std::span<const uint8_t, 6> Timestamp()      const { return timestamp; }
  char                        EventCode()      const { return eventCode; }

private:
  char     msgType;
  uint16_t stockLocate;
  uint16_t trackingNumber;
  uint8_t  timestamp[6];
  char     eventCode;  // 'O'=start of messages, 'S'=system hours, 'Q'=market hours,
                       // 'M'=market close, 'E'=end system hours, 'C'=end of messages
};
static_assert(sizeof(ItchSystemEvent) == 12, "Size mismatch with ITCH 5.0 Spec");
#pragma pack(pop)

// 'R' — Stock Directory
#pragma pack(push, 1)
struct ItchStockDirectory
{
public:
  uint16_t                    StockLocate()              const { return __builtin_bswap16(stockLocate); }
  uint16_t                    TrackingNumber()           const { return __builtin_bswap16(trackingNumber); }
  std::span<const uint8_t, 6> Timestamp()                const { return timestamp; }
  const char*                 Stock()                    const { return stock; }
  char                        MarketCategory()           const { return marketCategory; }
  char                        FinancialStatusIndicator() const { return financialStatusIndicator; }
  uint32_t                    RoundLotSize()             const { return __builtin_bswap32(roundLotSize); }
  char                        RoundLotsOnly()            const { return roundLotsOnly; }
  char                        IssueClassification()      const { return issueClassification; }
  const char*                 IssueSubType()             const { return issueSubType; }
  char                        Authenticity()             const { return authenticity; }
  char                        ShortSaleThreshold()       const { return shortSaleThreshold; }
  char                        IpoFlag()                  const { return ipoFlag; }
  char                        LuldReferencePriceTier()   const { return luldReferencePriceTier; }
  char                        EtpFlag()                  const { return etpFlag; }
  uint32_t                    EtpLeverageFactor()        const { return __builtin_bswap32(etpLeverageFactor); }
  char                        InverseIndicator()         const { return inverseIndicator; }

private:
  char     msgType;
  uint16_t stockLocate;
  uint16_t trackingNumber;
  uint8_t  timestamp[6];
  char     stock[8];                  // Right-padded with spaces
  char     marketCategory;            // 'Q'=Nasdaq, 'N'=NYSE, 'A'=NYSE MKT, etc.
  char     financialStatusIndicator;
  uint32_t roundLotSize;
  char     roundLotsOnly;
  char     issueClassification;
  char     issueSubType[2];
  char     authenticity;
  char     shortSaleThreshold;
  char     ipoFlag;
  char     luldReferencePriceTier;
  char     etpFlag;
  uint32_t etpLeverageFactor;
  char     inverseIndicator;
};
static_assert(sizeof(ItchStockDirectory) == 39, "Size mismatch with ITCH 5.0 Spec");
#pragma pack(pop)

#pragma pack(push, 1)
struct ItchAddOrder
{
public:
  uint16_t                    StockLocate()          const { return __builtin_bswap16(stockLocate); }
  uint16_t                    TrackingNumber()       const { return __builtin_bswap16(trackingNumber); }
  std::span<const uint8_t, 6> Timestamp()            const { return timestamp; }
  uint64_t                    OrderReferenceNumber() const { return __builtin_bswap64(orderReferenceNumber); }
  char                        BuyOrSellIndicator()   const { return buyOrSellIndicator; }
  uint32_t                    Shares()               const { return __builtin_bswap32(shares); }
  const char*                 StockName()            const { return stockName; }
  uint32_t                    Price()                const { return __builtin_bswap32(price); }

private:
  char     msgType;
  uint16_t stockLocate;          // Security identifier
  uint16_t trackingNumber;       // Nasdaq internal tracking number
  uint8_t  timestamp[6];         // ns since midnight (big-endian, reconstruct to uint64_t)
  uint64_t orderReferenceNumber;
  char     buyOrSellIndicator;   // 'B' or 'S'
  uint32_t shares;
  char     stockName[8];         // Right-padded with spaces
  uint32_t price;                // Fixed-point, divide by 10'000
};
static_assert(sizeof(ItchAddOrder) == 36, "Size mismatch with ITCH 5.0 Spec");
#pragma pack(pop)

// 'F' — Add Order with MPID Attribution (identical to ItchAddOrder + 4-byte attribution)
#pragma pack(push, 1)
struct ItchAddOrderMpid
{
public:
  uint16_t                    StockLocate()          const { return __builtin_bswap16(stockLocate); }
  uint16_t                    TrackingNumber()       const { return __builtin_bswap16(trackingNumber); }
  std::span<const uint8_t, 6> Timestamp()            const { return timestamp; }
  uint64_t                    OrderReferenceNumber() const { return __builtin_bswap64(orderReferenceNumber); }
  char                        BuyOrSellIndicator()   const { return buyOrSellIndicator; }
  uint32_t                    Shares()               const { return __builtin_bswap32(shares); }
  const char*                 StockName()            const { return stockName; }
  uint32_t                    Price()                const { return __builtin_bswap32(price); }
  const char*                 Attribution()          const { return attribution; }

private:
  char     msgType;
  uint16_t stockLocate;
  uint16_t trackingNumber;
  uint8_t  timestamp[6];
  uint64_t orderReferenceNumber;
  char     buyOrSellIndicator;
  uint32_t shares;
  char     stockName[8];
  uint32_t price;
  char     attribution[4];          // Market participant identifier
};
static_assert(sizeof(ItchAddOrderMpid) == 40, "Size mismatch with ITCH 5.0 Spec");
#pragma pack(pop)

// 'U' — Order Replace
#pragma pack(push, 1)
struct ItchOrderReplace
{
public:
  uint16_t                    StockLocate()                  const { return __builtin_bswap16(stockLocate); }
  uint16_t                    TrackingNumber()               const { return __builtin_bswap16(trackingNumber); }
  std::span<const uint8_t, 6> Timestamp()                    const { return timestamp; }
  uint64_t                    OriginalOrderReferenceNumber() const { return __builtin_bswap64(originalOrderReferenceNumber); }
  uint64_t                    NewOrderReferenceNumber()      const { return __builtin_bswap64(newOrderReferenceNumber); }
  uint32_t                    Shares()                       const { return __builtin_bswap32(shares); }
  uint32_t                    Price()                        const { return __builtin_bswap32(price); }

private:
  char     msgType;
  uint16_t stockLocate;
  uint16_t trackingNumber;
  uint8_t  timestamp[6];
  uint64_t originalOrderReferenceNumber;
  uint64_t newOrderReferenceNumber;
  uint32_t shares;
  uint32_t price;
};
static_assert(sizeof(ItchOrderReplace) == 35, "Size mismatch with ITCH 5.0 Spec");
#pragma pack(pop)

#pragma pack(push, 1)
struct ItchOrderCancel
{
public:
  uint16_t                    StockLocate()          const { return __builtin_bswap16(stockLocate); }
  uint16_t                    TrackingNumber()       const { return __builtin_bswap16(trackingNumber); }
  std::span<const uint8_t, 6> Timestamp()            const { return timestamp; }
  uint64_t                    OrderReferenceNumber() const { return __builtin_bswap64(orderReferenceNumber); }
  uint32_t                    CancelledShares()      const { return __builtin_bswap32(cancelledShares); }

private:
  char     msgType;
  uint16_t stockLocate;
  uint16_t trackingNumber;
  uint8_t  timestamp[6];
  uint64_t orderReferenceNumber;
  uint32_t cancelledShares;
};
static_assert(sizeof(ItchOrderCancel) == 23, "Size mismatch with ITCH 5.0 Spec");
#pragma pack(pop)

#pragma pack(push, 1)
struct ItchOrderExecuted
{
public:
  uint16_t                    StockLocate()          const { return __builtin_bswap16(stockLocate); }
  uint16_t                    TrackingNumber()       const { return __builtin_bswap16(trackingNumber); }
  std::span<const uint8_t, 6> Timestamp()            const { return timestamp; }
  uint64_t                    OrderReferenceNumber() const { return __builtin_bswap64(orderReferenceNumber); }
  uint32_t                    ExecutedShares()       const { return __builtin_bswap32(executedShares); }
  uint64_t                    MatchNumber()          const { return __builtin_bswap64(matchNumber); }

private:
  char     msgType;
  uint16_t stockLocate;
  uint16_t trackingNumber;
  uint8_t  timestamp[6];
  uint64_t orderReferenceNumber;
  uint32_t executedShares;
  uint64_t matchNumber;
};
static_assert(sizeof(ItchOrderExecuted) == 31, "Size mismatch with ITCH 5.0 Spec");
#pragma pack(pop)

#pragma pack(push, 1)
struct ItchOrderDelete
{
public:
  uint16_t                    StockLocate()          const { return __builtin_bswap16(stockLocate); }
  uint16_t                    TrackingNumber()       const { return __builtin_bswap16(trackingNumber); }
  std::span<const uint8_t, 6> Timestamp()            const { return timestamp; }
  uint64_t                    OrderReferenceNumber() const { return __builtin_bswap64(orderReferenceNumber); }

private:
  char     msgType;
  uint16_t stockLocate;
  uint16_t trackingNumber;
  uint8_t  timestamp[6];
  uint64_t orderReferenceNumber;
};
static_assert(sizeof(ItchOrderDelete) == 19, "Size mismatch with ITCH 5.0 Spec");
#pragma pack(pop)

// 'C' — Order Executed With Price
#pragma pack(push, 1)
struct ItchOrderExecutedWithPrice
{
public:
  uint16_t                    StockLocate()          const { return __builtin_bswap16(stockLocate); }
  uint16_t                    TrackingNumber()       const { return __builtin_bswap16(trackingNumber); }
  std::span<const uint8_t, 6> Timestamp()            const { return timestamp; }
  uint64_t                    OrderReferenceNumber() const { return __builtin_bswap64(orderReferenceNumber); }
  uint32_t                    ExecutedShares()       const { return __builtin_bswap32(executedShares); }
  uint64_t                    MatchNumber()          const { return __builtin_bswap64(matchNumber); }
  char                        Printable()            const { return printable; }
  uint32_t                    ExecutionPrice()       const { return __builtin_bswap32(executionPrice); }

private:
  char     msgType;
  uint16_t stockLocate;
  uint16_t trackingNumber;
  uint8_t  timestamp[6];
  uint64_t orderReferenceNumber;
  uint32_t executedShares;
  uint64_t matchNumber;
  char     printable;        // 'Y' = printable to tape, 'N' = non-printable
  uint32_t executionPrice;   // Fixed-point, divide by 10'000; may differ from resting price
};
static_assert(sizeof(ItchOrderExecutedWithPrice) == 36, "Size mismatch with ITCH 5.0 Spec");
#pragma pack(pop)
