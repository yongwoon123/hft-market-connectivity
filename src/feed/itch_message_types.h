#pragma once

#include <cstdint>
#include <span>

// Fields are intentionally duplicated across structs rather than extracted into a common base.
// Inheritance breaks standard-layout, making offsetof on derived members undefined behaviour
// and preventing reliable static_assert layout validation against the wire format.
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
