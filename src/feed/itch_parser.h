#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <vector>

#include "infra/latency_recorder.h"

template <typename THandler>
class ItchParser
{
public:
  ItchParser(const std::filesystem::path& path, THandler& handler, LatencyRecorder* recorder = nullptr);
  ~ItchParser();

  // progress callback receives (bytes_read, total_bytes); called every 1M messages
  void Parse(uint64_t limit = 0, std::function<void(uint64_t, uint64_t)> progress = nullptr);

private:
  void ProcessMessage(const char* data, uint8_t msgType);
  bool Refill();

  static constexpr size_t kBufSize = 8 * 1024 * 1024;
  static constexpr size_t kMinHeadroom = 51;  // 2-byte length header + 49-byte max body (NOII)

  int mFileDescriptor = -1;
  uint64_t mFileSize = 0;
  std::vector<char> mBuffer;
  size_t mCursor = 0;
  size_t mFillEnd = 0;
  LatencyRecorder* mRecorder = nullptr;

  THandler& mItchHandler;
};

#include "itch_parser.hpp"
