#include <iomanip>
#include <iostream>

#include "book/book_manager.h"
#include "feed/itch_handler.h"
#include "feed/itch_parser.h"
#include "infra/clock.h"

static void printProgress(uint64_t done, uint64_t total)
{
  static constexpr int kBarWidth = 40;
  float pct = total > 0 ? static_cast<float>(done) / static_cast<float>(total) : 0.0f;
  int filled = static_cast<int>(pct * kBarWidth);

  std::cout << "\r[";
  for (int i = 0; i < kBarWidth; ++i) std::cout << (i < filled ? '=' : (i == filled ? '>' : ' '));
  std::cout << "] " << std::setw(3) << static_cast<int>(pct * 100) << "%" << std::flush;
}

int main()
{
  std::cout << "hft-engine starting\n";

  BookManager bookManager;
  ItchHandler handler{bookManager};
  ItchParser parser{"data/sample_feed/01302019.NASDAQ_ITCH50", handler};

  uint64_t start = infra::now_ns();
  parser.Parse(0, printProgress);
  std::cout << "\n";
  uint64_t elapsed_ns = infra::now_ns() - start;

  double elapsed_s = static_cast<double>(elapsed_ns) / 1e9;
  std::cout << "Elapsed: " << elapsed_s << "s\n";

  // Post-parse stats
  uint64_t totalBooks   = 0;
  uint64_t totalErrors  = 0;
  uint64_t twoSided     = 0;
  uint64_t inverted     = 0;

  static constexpr int     kSampleSize = 5;
  static constexpr uint16_t kMaxLocate = 8000;
  int samplesFound = 0;

  for (uint16_t loc = 1; loc <= kMaxLocate; ++loc)
  {
    const OrderBook* book = bookManager.GetBook(loc);
    if (!book) continue;
    ++totalBooks;
    totalErrors += book->ErrorCount();

    auto bid = book->BestBid();
    auto ask = book->BestAsk();
    if (!bid || !ask) continue;
    ++twoSided;
    if (bid->first >= ask->first) { ++inverted; continue; }

    if (samplesFound < kSampleSize)
    {
      std::cout << "  locate=" << loc
                << " bid=" << bid->first / 10000 << "." << (bid->first % 10000) / 100 << "x" << bid->second
                << " ask=" << ask->first / 10000 << "." << (ask->first % 10000) / 100 << "x" << ask->second
                << "\n";
      ++samplesFound;
    }
  }

  std::cout << "Books:    " << totalBooks  << "\n"
            << "2-sided:  " << twoSided    << " (" << inverted << " inverted)\n"
            << "Errors:   " << totalErrors << "\n";

  if (samplesFound == 0)
  {
    std::cerr << "ERROR: no book with a valid spread found\n";
    return 1;
  }

  return 0;
}
