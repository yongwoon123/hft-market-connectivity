#include <iostream>

#include "feed/itch_handler.h"
#include "feed/itch_parser.h"

int main()
{
  std::cout << "hft-engine starting\n";

  ItchHandler handler;
  ItchParser parser{"data/sample_feed/01302019.NASDAQ_ITCH50", handler};
  parser.Parse(10'000'000);
  
  return 0;
}
