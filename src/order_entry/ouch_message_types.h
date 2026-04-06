#pragma once

#include <cstdint>
#include <cstring>

// Fields are intentionally duplicated across structs rather than extracted into a common base.
// Inheritance breaks standard-layout, making offsetof on derived members undefined behaviour
// and preventing reliable static_assert layout validation against the wire format.

// Note: a production encoder would write fields directly into a pre-allocated byte buffer at
// known offsets, avoiding the packed struct entirely. This approach is sufficient for now.

// ---------------------------------------------------------------------------
// Outbound (client -> NASDAQ)
// ---------------------------------------------------------------------------

// 'O' — Enter Order (Section 2.1)
#pragma pack(push, 1)
struct OuchEnterOrder
{
  OuchEnterOrder(uint32_t    userRefNum,
                 char        side,
                 uint32_t    quantity,
                 const char* orderSymbol,
                 uint64_t    price,
                 char        timeInForce,
                 char        display,
                 char        capacity,
                 char        interMarketSweepEligibility,
                 char        crossType,
                 const char* orderClOrdId)
      : type('O')
      , userRefNum(__builtin_bswap32(userRefNum))
      , side(side)
      , quantity(__builtin_bswap32(quantity))
      , price(__builtin_bswap64(price))
      , timeInForce(timeInForce)
      , display(display)
      , capacity(capacity)
      , interMarketSweepEligibility(interMarketSweepEligibility)
      , crossType(crossType)
      , appendageLength(__builtin_bswap16(0))
  {
    memset(symbol, ' ', 8);
    memcpy(symbol, orderSymbol, strnlen(orderSymbol, 8));
    memset(clOrdId, ' ', 14);
    memcpy(clOrdId, orderClOrdId, strnlen(orderClOrdId, 14));
  }

  char        GetType()                        const { return type; }
  uint32_t    GetUserRefNum()                  const { return __builtin_bswap32(userRefNum); }
  char        GetSide()                        const { return side; }
  uint32_t    GetQuantity()                    const { return __builtin_bswap32(quantity); }
  const char* GetSymbol()                      const { return symbol; }
  uint64_t    GetPrice()                       const { return __builtin_bswap64(price); }
  char        GetTimeInForce()                 const { return timeInForce; }
  char        GetDisplay()                     const { return display; }
  char        GetCapacity()                    const { return capacity; }
  char        GetInterMarketSweepEligibility() const { return interMarketSweepEligibility; }
  char        GetCrossType()                   const { return crossType; }
  const char* GetClOrdId()                     const { return clOrdId; }
  uint16_t    GetAppendageLength()             const { return __builtin_bswap16(appendageLength); }

private:
  char     type;
  uint32_t userRefNum;
  char     side;
  uint32_t quantity;
  char     symbol[8];
  uint64_t price;
  char     timeInForce;
  char     display;
  char     capacity;
  char     interMarketSweepEligibility;
  char     crossType;
  char     clOrdId[14];
  uint16_t appendageLength;
};
static_assert(sizeof(OuchEnterOrder) == 47, "Size mismatch with OUCH 5.0 Spec");
// Field offsets are verified by byte-level assertions in test_ouch_encoder.cpp.
#pragma pack(pop)

// 'X' — Cancel Order Request (Section 2.3)
#pragma pack(push, 1)
struct OuchCancelOrder
{
  OuchCancelOrder(uint32_t userRefNum, uint32_t quantity)
      : type('X')
      , userRefNum(__builtin_bswap32(userRefNum))
      , quantity(__builtin_bswap32(quantity))
      , appendageLength(__builtin_bswap16(0))
  {}

  char     GetType()            const { return type; }
  uint32_t GetUserRefNum()      const { return __builtin_bswap32(userRefNum); }
  uint32_t GetQuantity()        const { return __builtin_bswap32(quantity); }
  uint16_t GetAppendageLength() const { return __builtin_bswap16(appendageLength); }

private:
  char     type;
  uint32_t userRefNum;
  uint32_t quantity;
  uint16_t appendageLength;
};
static_assert(sizeof(OuchCancelOrder) == 11, "Size mismatch with OUCH 5.0 Spec");
#pragma pack(pop)

// ---------------------------------------------------------------------------
// Inbound (NASDAQ -> client)
// ---------------------------------------------------------------------------

// 'A' — Order Accepted (Section 3.2)
#pragma pack(push, 1)
struct OuchOrderAccepted
{
  char        GetType()                        const { return type; }
  uint64_t    GetTimestamp()                   const { return __builtin_bswap64(timestamp); }
  uint32_t    GetUserRefNum()                  const { return __builtin_bswap32(userRefNum); }
  char        GetSide()                        const { return side; }
  uint32_t    GetQuantity()                    const { return __builtin_bswap32(quantity); }
  const char* GetSymbol()                      const { return symbol; }
  uint64_t    GetPrice()                       const { return __builtin_bswap64(price); }
  char        GetTimeInForce()                 const { return timeInForce; }
  char        GetDisplay()                     const { return display; }
  uint64_t    GetOrderReferenceNumber()        const { return __builtin_bswap64(orderReferenceNumber); }
  char        GetCapacity()                    const { return capacity; }
  char        GetInterMarketSweepEligibility() const { return interMarketSweepEligibility; }
  char        GetCrossType()                   const { return crossType; }
  char        GetOrderState()                  const { return orderState; }
  const char* GetClOrdId()                     const { return clOrdId; }
  uint16_t    GetAppendageLength()             const { return __builtin_bswap16(appendageLength); }

private:
  char     type;
  uint64_t timestamp;
  uint32_t userRefNum;
  char     side;
  uint32_t quantity;
  char     symbol[8];
  uint64_t price;
  char     timeInForce;
  char     display;
  uint64_t orderReferenceNumber;
  char     capacity;
  char     interMarketSweepEligibility;
  char     crossType;
  char     orderState;
  char     clOrdId[14];
  uint16_t appendageLength;
};
static_assert(sizeof(OuchOrderAccepted) == 64, "Size mismatch with OUCH 5.0 Spec");
#pragma pack(pop)

// 'J' — Order Rejected (Section 3.8)
#pragma pack(push, 1)
struct OuchOrderRejected
{
  char        GetType()            const { return type; }
  uint64_t    GetTimestamp()       const { return __builtin_bswap64(timestamp); }
  uint32_t    GetUserRefNum()      const { return __builtin_bswap32(userRefNum); }
  uint16_t    GetReason()          const { return __builtin_bswap16(reason); }
  const char* GetClOrdId()         const { return clOrdId; }
  uint16_t    GetAppendageLength() const { return __builtin_bswap16(appendageLength); }

private:
  char     type;
  uint64_t timestamp;
  uint32_t userRefNum;
  uint16_t reason;
  char     clOrdId[14];
  uint16_t appendageLength;
};
static_assert(sizeof(OuchOrderRejected) == 31, "Size mismatch with OUCH 5.0 Spec");
#pragma pack(pop)

// 'E' — Order Executed (Section 3.6)
#pragma pack(push, 1)
struct OuchOrderExecuted
{
  char     GetType()            const { return type; }
  uint64_t GetTimestamp()       const { return __builtin_bswap64(timestamp); }
  uint32_t GetUserRefNum()      const { return __builtin_bswap32(userRefNum); }
  uint32_t GetQuantity()        const { return __builtin_bswap32(quantity); }
  uint64_t GetPrice()           const { return __builtin_bswap64(price); }
  char     GetLiquidityFlag()   const { return liquidityFlag; }
  uint64_t GetMatchNumber()     const { return __builtin_bswap64(matchNumber); }
  uint16_t GetAppendageLength() const { return __builtin_bswap16(appendageLength); }

private:
  char     type;
  uint64_t timestamp;
  uint32_t userRefNum;
  uint32_t quantity;
  uint64_t price;
  char     liquidityFlag;
  uint64_t matchNumber;
  uint16_t appendageLength;
};
static_assert(sizeof(OuchOrderExecuted) == 36, "Size mismatch with OUCH 5.0 Spec");
#pragma pack(pop)

// 'C' — Order Canceled (Section 3.4)
#pragma pack(push, 1)
struct OuchOrderCanceled
{
  char     GetType()            const { return type; }
  uint64_t GetTimestamp()       const { return __builtin_bswap64(timestamp); }
  uint32_t GetUserRefNum()      const { return __builtin_bswap32(userRefNum); }
  uint32_t GetQuantity()        const { return __builtin_bswap32(quantity); }
  char     GetReason()          const { return reason; }
  uint16_t GetAppendageLength() const { return __builtin_bswap16(appendageLength); }

private:
  char     type;
  uint64_t timestamp;
  uint32_t userRefNum;
  uint32_t quantity;
  char     reason;
  uint16_t appendageLength;
};
static_assert(sizeof(OuchOrderCanceled) == 20, "Size mismatch with OUCH 5.0 Spec");
#pragma pack(pop)
