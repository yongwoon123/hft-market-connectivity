#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>

// Single-threaded use only. SetT0 and RecordT1 are called synchronously within
// the same ProcessMessage invocation, so mT0 cannot be overwritten between them.
struct LatencyRecorder
{
  void SetT0(uint64_t t) { mT0 = t; }
  void RecordT1(uint64_t t1)
  {
    if (mCount < kCapacity)
    {
      mSamples[mCount++] = t1 - mT0;
    }
  }

  void PrintPercentiles() const
  {
    if (mCount == 0)
    {
      std::cout << "LatencyRecorder: no samples\n";
      return;
    }

    // sort a mutable copy - PrintPercentiles is const, samples are read-only here
    std::array<uint64_t, kCapacity> sorted = mSamples;
    std::sort(sorted.begin(), sorted.begin() + mCount);

    auto pct = [&](size_t num, size_t den) -> uint64_t { return sorted[(mCount - 1) * num / den]; };

    std::cout << "Latency percentiles (" << mCount << " samples):\n"
              << "  p50:   " << pct(50, 100) << " ns\n"
              << "  p95:   " << pct(95, 100) << " ns\n"
              << "  p99:   " << pct(99, 100) << " ns\n"
              << "  p99.9: " << pct(999, 1000) << " ns\n"
              << "  max:   " << sorted[mCount - 1] << " ns\n";
  }

  uint64_t SampleCount() const { return mCount; }

private:
  static constexpr size_t kCapacity = 1 << 16;
  std::array<uint64_t, kCapacity> mSamples{};
  uint64_t mT0 = 0;
  size_t mCount = 0;
};
