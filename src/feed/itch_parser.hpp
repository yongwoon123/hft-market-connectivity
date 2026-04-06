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
  mFileSize = std::filesystem::file_size(path);
}

template <typename THandler>
void ItchParser<THandler>::Parse(uint64_t limit, std::function<void(uint64_t, uint64_t)> progress)
{
  static constexpr uint64_t kProgressInterval = 1'000'000;

  uint64_t count = 0;

  std::vector<char> buffer(256);
  while (mFileHandler.good() && (limit == 0 || count < limit))
  {
    uint16_t msgLength;
    mFileHandler.read(reinterpret_cast<char*>(&msgLength), 2);
    if (!mFileHandler.good())
    {
      break;
    }
    msgLength = __builtin_bswap16(msgLength);

    buffer.resize(msgLength);
    mFileHandler.read(buffer.data(), msgLength);

    uint8_t msgType = static_cast<uint8_t>(buffer[0]);
    ProcessMessage(buffer.data(), msgType);

    if (progress && ++count % kProgressInterval == 0)
    {
      progress(static_cast<uint64_t>(mFileHandler.tellg()), mFileSize);
    }
  }

  if (progress)
  {
    progress(mFileSize, mFileSize);
  }
}

template <typename THandler>
void ItchParser<THandler>::ProcessMessage(const char* iter, const uint8_t msgType)
{
  switch (msgType)
  {
    case 'A': mItchHandler.template Dispatch<ItchAddOrder>(iter); break;
    case 'F': mItchHandler.template Dispatch<ItchAddOrderMpid>(iter); break;
    case 'U': mItchHandler.template Dispatch<ItchOrderReplace>(iter); break;
    case 'X': mItchHandler.template Dispatch<ItchOrderCancel>(iter); break;
    case 'E': mItchHandler.template Dispatch<ItchOrderExecuted>(iter); break;
    case 'C': mItchHandler.template Dispatch<ItchOrderExecutedWithPrice>(iter); break;
    case 'D': mItchHandler.template Dispatch<ItchOrderDelete>(iter); break;

    default:
    {
      // Unknown message type
      // Should log an error
      break;
    }
  }
}
