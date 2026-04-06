#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#include <vector>

#include "book/book_manager.h"
#include "feed/itch_handler.h"
#include "feed/itch_message_types.h"
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

struct PreOpenResult
{
  std::unordered_set<uint16_t> validLocates;
  uint16_t                     maxLocate = 0;
};

static PreOpenResult runPass1(const std::filesystem::path& path)
{
  PreOpenResult result;
  std::ifstream file(path, std::ios::binary);
  std::vector<char> buf(64);

  while (file.good())
  {
    uint16_t msgLen;
    file.read(reinterpret_cast<char*>(&msgLen), 2);
    if (!file.good()) break;
    msgLen = __builtin_bswap16(msgLen);

    buf.resize(msgLen);
    file.read(buf.data(), msgLen);
    if (!file.good()) break;

    const uint8_t msgType = static_cast<uint8_t>(buf[0]);

    if (msgType == 'R')
    {
      ItchStockDirectory msg;
      std::memcpy(&msg, buf.data(), sizeof(ItchStockDirectory));
      const uint16_t locate = msg.StockLocate();
      result.validLocates.insert(locate);
      if (locate > result.maxLocate) result.maxLocate = locate;
    }
    else if (msgType == 'S')
    {
      ItchSystemEvent msg;
      std::memcpy(&msg, buf.data(), sizeof(ItchSystemEvent));
      if (msg.EventCode() == 'Q')
      {
        break;
      }
    }
  }

  return result;
}

int main()
{
  static constexpr const char* kFeedPath = "data/sample_feed/01302019.NASDAQ_ITCH50";
  static constexpr int         kSampleSize = 5;

  std::cout << "hft-engine starting\n";
  std::cout << "Pass 1: scanning pre-open messages...\n";

  const PreOpenResult preOpen = runPass1(kFeedPath);

  std::cout << "  Locates: " << preOpen.validLocates.size()
            << "  max=" << preOpen.maxLocate << "\n";

  BookManager bookManager(preOpen.validLocates, preOpen.maxLocate);
  ItchHandler handler{bookManager};
  ItchParser  parser{kFeedPath, handler};

  auto printBookStats = [&](const char* label)
  {
    uint64_t totalBooks  = 0;
    uint64_t totalErrors = 0;
    uint64_t twoSided    = 0;
    uint64_t inverted    = 0;
    int      samples     = 0;

    std::cout << "\n--- " << label << " ---\n";
    for (uint16_t loc : bookManager.ValidLocates())
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
        std::cout << "  locate=" << loc
                  << " bid=" << bid->first / 10000 << "." << std::setw(2) << std::setfill('0') << (bid->first % 10000) / 100 << "x" << bid->second
                  << " ask=" << ask->first / 10000 << "." << std::setw(2) << std::setfill('0') << (ask->first % 10000) / 100 << "x" << ask->second
                  << "\n";
        ++samples;
      }
    }

    std::cout << "Books:   " << totalBooks  << "\n"
              << "2-sided: " << twoSided    << " (" << inverted << " inverted)\n"
              << "Errors:  " << totalErrors << "\n";
  };

  bool snapshotTaken = false;
  auto progress = [&](uint64_t done, uint64_t total)
  {
    printProgress(done, total);
    if (!snapshotTaken && total > 0 && done >= total * 6 / 10)
    {
      snapshotTaken = true;
      printBookStats("60% snapshot");
      std::cout << std::flush;
    }
  };

  std::cout << "Pass 2: parsing full file (pre-open orders must be in book before trading starts)...\n";

  uint64_t start = infra::now_ns();
  parser.Parse(0, progress);
  std::cout << "\n";
  uint64_t elapsed_ns = infra::now_ns() - start;

  std::cout << "Elapsed: " << static_cast<double>(elapsed_ns) / 1e9 << "s\n";

  printBookStats("end of day");

  return 0;
}
