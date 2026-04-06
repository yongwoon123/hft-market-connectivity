#pragma once

#include <cstdint>
#include <time.h>

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
}
