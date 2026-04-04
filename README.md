# hft-market-connectivity

A market connectivity layer in C++20 implementing the full exchange integration pipeline:

- Parse a real exchange protocol (NASDAQ ITCH 5.0) into structured market data
- Maintain a correct, fast order book from that data
- Send orders via a real exchange protocol (NASDAQ OUCH 5.0)

End-to-end flow: feed event → book update → order sent → acknowledgement received.

```
ITCH connector  →  Order Book  →  OUCH connector
  (receive)          (state)         (send)
```

## Status

| Component | Status |
|-----------|--------|
| ITCH 5.0 file parser | Done (4 message types: Add, Cancel, Execute, Delete) |
| Order book | Planned |
| OUCH 5.0 order entry | Planned |
| Live UDP feed ingestion | Planned |
| End-to-end latency instrumentation | Planned |

Validated against real NASDAQ historical ITCH data.

## Build

```bash
cmake -B build -G Ninja && cmake --build build
./build/src/hft-engine
```

## Protocols

| Protocol | Transport | Spec | Sample Data |
|----------|-----------|------|-------------|
| NASDAQ ITCH 5.0 | MoldUDP64 (UDP) | [ITCH 5.0 Spec](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf) | [NASDAQ Historical Files](https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/) |
| NASDAQ OUCH 5.0 | SoupBinTCP (TCP) | [OUCH 5.0 Spec](https://nasdaqtrader.com/content/technicalsupport/specifications/TradingProducts/Ouch5.0.pdf) | — |

**Wire format notes:**
- All integers are big-endian — byte-swapped on read for x86
- Prices are fixed-point integers with 4 implied decimal places (e.g. `500100` = `$50.0100`)
- Timestamps are nanoseconds since midnight
- Historical ITCH files are a flat length-prefixed stream — MoldUDP64 packet framing applies only to the live UDP feed
- Raw POSIX sockets only — no networking library wrappers

## Design

**Templated handler pattern** — the parser is templated on a handler type. Message dispatch is resolved at compile time with zero virtual call overhead. Handlers are swappable without touching the parser, making test handlers trivial to inject.

**Lazy byteswap accessors** — wire-format structs store fields in big-endian as received. Each field exposes a typed accessor that byteswaps on read, so only accessed fields pay the conversion cost.

## Architecture

```
src/
├── feed/          ← ITCH 5.0 parser and message types
├── book/          ← order book
├── order_entry/   ← OUCH 5.0 encoder and dispatcher
├── strategy/      ← simple interactor stub
├── network/       ← POSIX socket I/O
└── infra/         ← nanosecond clock, logging
```
