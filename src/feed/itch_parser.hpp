#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include "infra/clock.h"
#include "itch_message_types.h"
#include "itch_parser.h"  // clangd: provides class declaration for standalone analysis

template <typename THandler>
ItchParser<THandler>::ItchParser(const std::filesystem::path& path, THandler& handler, LatencyRecorder* recorder)
    : mFileDescriptor(::open(path.c_str(), O_RDONLY))
    , mBuffer(ItchParser::kBufSize)
    , mRecorder(recorder)
    , mItchHandler(handler)
{
  if (mFileDescriptor < 0)
  {
    throw std::runtime_error("ItchParser: failed to open " + path.string());
  }

  struct stat st;
  if (::fstat(mFileDescriptor, &st) == 0)
  {
    mFileSize = static_cast<uint64_t>(st.st_size);
  }
}

template <typename THandler>
ItchParser<THandler>::~ItchParser()
{
  if (mFileDescriptor >= 0)
  {
    ::close(mFileDescriptor);
  }
}

template <typename THandler>
bool ItchParser<THandler>::Refill()
{
  const size_t remaining = mFillEnd - mCursor;
  if (remaining > 0)
  {
    std::memmove(mBuffer.data(), mBuffer.data() + mCursor, remaining);
  }

  mCursor = 0;
  mFillEnd = remaining;

  ssize_t n;
  do
  {
    n = ::read(mFileDescriptor, mBuffer.data() + remaining, ItchParser::kBufSize - remaining);
  } while (n < 0 && errno == EINTR);

  if (n > 0)
  {
    mFillEnd += static_cast<size_t>(n);
  }

  return mFillEnd > 0;
}

template <typename THandler>
void ItchParser<THandler>::Parse(uint64_t limit, std::function<void(uint64_t, uint64_t)> progress)
{
  static constexpr uint64_t kProgressInterval = 1'000'000;

  uint64_t count = 0;
  uint64_t consumed = 0;

  while (limit == 0 || count < limit)
  {
    if (mFillEnd - mCursor < ItchParser::kMinHeadroom)
    {
      if (!Refill() || mFillEnd - mCursor < 2) break;
    }

    uint16_t msgLength;
    std::memcpy(&msgLength, mBuffer.data() + mCursor, 2);
    msgLength = __builtin_bswap16(msgLength);
    mCursor += 2;

    if (mFillEnd - mCursor < msgLength) break;  // truncated message at EOF

    const uint8_t msgType = static_cast<uint8_t>(mBuffer[mCursor]);
    ProcessMessage(mBuffer.data() + mCursor, msgType);

    consumed += 2 + msgLength;
    mCursor += msgLength;

    if (progress && ++count % kProgressInterval == 0)
    {
      progress(consumed, mFileSize);
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
  if (mRecorder) mRecorder->SetT0(infra::now_ns_monotonic());

  switch (msgType)
  {
    case 'A': mItchHandler.template Dispatch<ItchAddOrder>(iter); break;
    case 'F': mItchHandler.template Dispatch<ItchAddOrderMpid>(iter); break;
    case 'U': mItchHandler.template Dispatch<ItchOrderReplace>(iter); break;
    case 'X': mItchHandler.template Dispatch<ItchOrderCancel>(iter); break;
    case 'E': mItchHandler.template Dispatch<ItchOrderExecuted>(iter); break;
    case 'C': mItchHandler.template Dispatch<ItchOrderExecutedWithPrice>(iter); break;
    case 'D': mItchHandler.template Dispatch<ItchOrderDelete>(iter); break;

    default: break;
  }
}
