#pragma once

#include <cstddef>
#include <cstdint>

struct ISender
{
  virtual void Send(const uint8_t* data, size_t len) = 0;
  virtual ~ISender()                                 = default;
};
