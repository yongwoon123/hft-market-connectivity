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

  static constexpr int kSampleSize = 5;
  static constexpr uint16_t kMaxLocate = 8000;

  auto printBookStats = [&](const char* label)
  {
    uint64_t totalBooks = 0;
    uint64_t totalErrors = 0;
    uint64_t twoSided = 0;
    uint64_t inverted = 0;
    int samples = 0;

    std::cout << "\n--- " << label << " ---\n";
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
      if (bid->first >= ask->first)
      {
        ++inverted;
        continue;
      }

      if (samples < kSampleSize)
      {
        std::cout << "  locate=" << loc << " bid=" << bid->first / 10000 << "." << std::setw(2)
                  << std::setfill('0') << (bid->first % 10000) / 100 << "x" << bid->second
                  << " ask=" << ask->first / 10000 << "." << std::setw(2) << std::setfill('0')
                  << (ask->first % 10000) / 100 << "x" << ask->second << "\n";
        ++samples;
      }
    }

    std::cout << "Books:   " << totalBooks << "\n"
              << "2-sided: " << twoSided << " (" << inverted << " inverted)\n"
              << "Errors:  " << totalErrors << "\n";

    return samples;
  };

  bool snapshotTaken = false;
  auto progress = [&](uint64_t done, uint64_t total)
  {
    printProgress(done, total);
    if (!snapshotTaken && total > 0 && done >= total * 6 / 10)
    {
      snapshotTaken = true;
      printBookStats("60'%' snapshot");
      std::cout << std::flush;
    }
  };

  uint64_t start = infra::now_ns();
  parser.Parse(0, progress);
  std::cout << "\n";
  uint64_t elapsed_ns = infra::now_ns() - start;

  double elapsed_s = static_cast<double>(elapsed_ns) / 1e9;
  std::cout << "Elapsed: " << elapsed_s << "s\n";

  printBookStats("end of day");

  return 0;
}
