#pragma once
#include <cstring>
#include <vector>

#include "itch_message_types.h"
#include "itch_parser.h"  // clangd: provides class declaration for standalone analysis

template <typename THandler>
ItchParser<THandler>::ItchParser(const std::filesystem::path& path, THandler& tHandler)
    : mFileHandler(path, std::ios::binary)
    , mItchHandler(tHandler)
{
}

template <typename THandler>
void ItchParser<THandler>::Parse(uint64_t limit)
{
  uint64_t count = 0;

  while (mFileHandler.good() && (limit == 0 || count++ < limit))
  {
    uint16_t msgLength;
    mFileHandler.read(reinterpret_cast<char*>(&msgLength), 2);
    if (!mFileHandler.good())
    {
      break;
    }
    msgLength = __builtin_bswap16(msgLength);

    std::vector<char> buffer(msgLength);
    mFileHandler.read(buffer.data(), msgLength);

    uint8_t msgType = static_cast<uint8_t>(buffer[0]);
    ProcessMessage(buffer.data(), msgType);
  }
}

template <typename THandler>
void ItchParser<THandler>::ProcessMessage(const char* iter, const uint8_t msgType)
{
  switch (msgType)
  {
    case 'A': mItchHandler.template Dispatch<ItchAddOrder>(iter); break;
    case 'X': mItchHandler.template Dispatch<ItchOrderCancel>(iter); break;
    case 'E': mItchHandler.template Dispatch<ItchOrderExecuted>(iter); break;
    case 'D': mItchHandler.template Dispatch<ItchOrderDelete>(iter); break;

    default:
    {
      // Unknown message type
      // Should log an error
      break;
    }
  }
}
