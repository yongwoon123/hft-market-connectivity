#pragma once

#include <time.h>

#include <cstdint>

namespace infra
{
inline uint64_t now_ns()
{
  struct timespec ts;
  /*
  - `CLOCK_REALTIME`  - Wall clock, matches exchange timestamps
  - `CLOCK_MONOTONIC` - Measuring elapsed time and latency
  - `CLOCK_TAI`       - Used to avoid leap second issues
  */
  clock_gettime(CLOCK_REALTIME, &ts);
  return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL + ts.tv_nsec;
}

inline uint64_t now_ns_monotonic()
{
  struct timespec ts;
  /*
  - `CLOCK_REALTIME`  - Wall clock, matches exchange timestamps
  - `CLOCK_MONOTONIC` - Measuring elapsed time and latency
  - `CLOCK_TAI`       - Used to avoid leap second issues
  */
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<uint64_t>(ts.tv_sec) * 1'000'000'000ULL + ts.tv_nsec;
}

inline uint32_t nowSinceMidnight_ms()
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return static_cast<uint32_t>((ts.tv_sec % 86400) * 1000 + ts.tv_nsec / 1'000'000);
}
}  // namespace infra
