#pragma once

#include <filesystem>
#include <fstream>

template <typename THandler>
class ItchParser
{
public:
  ItchParser(const std::filesystem::path& path, THandler& tHandler);

  void Parse(uint64_t limit = 0);

private:
  void ProcessMessage(const char* iter, const uint8_t msgType);

  // Emulating socket
  std::ifstream mFileHandler;

  THandler& mItchHandler;
};

#include "itch_parser.hpp"
