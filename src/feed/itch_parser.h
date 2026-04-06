#pragma once

#include <filesystem>
#include <fstream>
#include <functional>

template <typename THandler>
class ItchParser
{
public:
  ItchParser(const std::filesystem::path& path, THandler& tHandler);

  // progress callback receives (bytes_read, total_bytes); called every 1M messages
  void Parse(uint64_t limit = 0, std::function<void(uint64_t, uint64_t)> progress = nullptr);

private:
  void ProcessMessage(const char* iter, const uint8_t msgType);

  // Emulating socket
  std::ifstream mFileHandler;
  uint64_t      mFileSize = 0;

  THandler& mItchHandler;
};

#include "itch_parser.hpp"
