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
| ITCH 5.0 file parser | Done (Add, AddMpid, Cancel, Execute, ExecuteWithPrice, Delete, Replace) |
| L3 Order book | Done — per-order flat sorted vector, level aggregation on query, AddOrder, CancelOrder, ExecuteOrder, DeleteOrder, ReplaceOrder, BidDepth, AskDepth |
| BookManager routing layer | Done — flat `vector<OrderBook>` indexed by stockLocate, two-pass init |
| End-to-end benchmark | Done — full NASDAQ file parse, 60% mid-day snapshot, book count/error/spread stats |
| OUCH 5.0 order entry | Done - SoupBinTCP framing, Enter Order + Cancel Order encoding, UserRefNum sequencing, ISender injection for testability |
| Strategy layer | Done - PassiveQuoter: single-symbol send-once passive bid, ItchHandler templatised on strategy type |
| End-to-end pipeline | Done - ITCH feed -> BookManager -> PassiveQuoter -> OUCH wire bytes, PrintSender for demo output |
| Live UDP feed ingestion | Planned |
| End-to-end latency instrumentation | Planned |
| PassiveQuoter order lifecycle (cancel, replace, fill reaction) | Planned |
| Hot path allocation audit | Planned |

Validated against real NASDAQ historical ITCH data (01302019).

## Build

```bash
# Debug (default for tests and clangd)
cmake --preset debug && cmake --build build/debug
./build/debug/tests/hft-tests

# Release (for benchmarking)
cmake --preset release && cmake --build build/release
./build/release/src/hft-engine
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

**OUCH outbound encoding** — outbound message structs take host-order values in their constructor and store wire-ready big-endian bytes internally. A stateless template encoder prepends the SoupBinTCP frame header and memcpy's the struct into a pre-allocated session buffer — no heap allocation on the send path. Struct sizes are verified against the spec with `static_assert`. The session owns a monotonic UserRefNum counter seeded from milliseconds since midnight, guaranteeing the strictly-increasing invariant across reconnects within a trading day. `ISender` is injected at construction, making the full encode-and-send path testable without a live TCP connection.

**Strategy wiring** — `ItchHandler` is templatised on a strategy type (`ItchHandler<TStrategy>`), mirroring the `ItchParser<THandler>` pattern. After each book mutation, the handler calls `strategy.OnBookUpdate(locate, book)` with the updated `OrderBook`. The strategy holds a reference to `OrderEntrySession` and decides whether to send an order. `PassiveQuoter` is a minimal send-once strategy: it watches a single symbol, waits for a valid two-sided market (bid < ask), and posts a passive bid at `bestBid + 1` tick. No virtual dispatch on the hot path.

**Two-pass initialisation** — Pass 1 scans the pre-open session for Stock Directory (`'R'`) messages, collecting all valid stockLocate codes before the first order arrives. `BookManager` is then pre-constructed as a flat `vector<OrderBook>` indexed directly by stockLocate — O(1) array access replacing the hash map lookup on every message. Pass 2 parses the full file from the beginning; pre-open orders are processed correctly before trading starts at `'Q'` (Start of Market Hours). This mirrors live production systems, where the process starts before open and is fully initialised before the first order message arrives.

## Performance

Profiled against a full 11 GB NASDAQ ITCH historical file (~80M messages, ~8000 symbols).
Build: `-O3 -g -fno-omit-frame-pointer`. Hardware counters via `perf stat`.

```
                     |   Baseline | Pass 1: I/O | Pass 2: L2 Book | Pass 3: L3 Book |
                     |            |      Buffer |       + Reserve |       + Reserve |
---------------------|------------|-------------|-----------------|-----------------|
Total time (s)       |      342.3 |       256.9 |           206.1 |           249.8 |
IPC                  |       0.30 |        0.26 |            0.37 |            0.42 |
LLC cache miss rate  |     62.55% |      55.84% |          57.08% |          36.68% |
L1d cache miss rate  |     10.38% |      16.42% |          14.53% |          36.94% |
dTLB miss rate       |      2.26% |       3.74% |           2.29% |           1.40% |
Branch mispredict    |      3.92% |       5.12% |           5.29% |           5.65% |
Instructions         |      320 B |       183 B |          173 B  |          321 B  |
```

**Pass 1** replaces `std::ifstream` (two calls per message) with an 8 MB read-ahead buffer
using `open(2)`/`read(2)`. Eliminates ~160M syscalls. 25% wall time reduction.

**Pass 2** replaces `std::map` with a flat `std::vector<PriceLevel>` storing aggregated
price levels only. `mOrders.reserve(256)` eliminates rehash overhead. Level lookups become
a binary search over a contiguous array — no pointer-chasing. Wall time drops 20% vs Pass 1.

**Pass 3** (final implementation) stores individual orders in a flat sorted
`std::vector<Order>` and aggregates to levels at query time. `mOrders.reserve(128)`
eliminates rehash overhead. LLC miss rate drops to 36% and dTLB pressure halves vs Pass 1
as pointer-chasing from the red-black tree is eliminated. Wall time is higher than Pass 2
because the book maintains per-order state - the correct structure for ITCH, where
cancel/execute/replace all reference individual order refs that must be tracked.

## Architecture

```
src/
├── feed/          ← ITCH 5.0 parser and message types
├── book/          ← order book
├── order_entry/   ← OUCH 5.0 encoder and dispatcher
├── strategy/      ← trading strategy (PassiveQuoter)
├── network/       ← POSIX socket I/O
└── infra/         ← nanosecond clock, logging
```
