#pragma once

#include <cstdint>
#include <cstring>

namespace OuchEncoder
{
inline constexpr char kUnsequenced = 'U';

template <char PacketType, typename T>
size_t Encode(const T& msg, uint8_t* buffer, size_t bufferLength)
{
  const size_t payloadLen = sizeof(T) + msg.GetAppendageLength();
  const size_t totalLen = 3 + payloadLen;
  const uint16_t packetLength = __builtin_bswap16(1 + payloadLen);

  if (totalLen > bufferLength) return 0;

  memcpy(buffer, &packetLength, 2);
  buffer[2] = PacketType;
  memcpy(buffer + 3, &msg, sizeof(T));
  return totalLen;
}
}  // namespace OuchEncoder
