#pragma once
#include <immintrin.h>
#include <sstream>
#include <string>
#include <cstdint>
#include <unistd.h>

namespace utils
{
struct PerfCounter
{
    double ticks_per_sec;
    PerfCounter()
    {
        ticks_per_sec = static_cast<double>(measure_ticks_per_sec());
    }
    std::string dt_to_string(uint64_t dt)
    {
        const auto dt_in_seconds = static_cast<double>(dt) / ticks_per_sec;
        const auto seconds = static_cast<uint64_t>(dt_in_seconds);
        const auto fraction = dt_in_seconds - seconds;
        const auto msec = static_cast<uint64_t>(fraction * 1000);
        const auto usec = static_cast<uint64_t>(fraction * 1000000) % 1000;
        std::ostringstream ss;
        if (seconds > 0) {
            ss << seconds << " s ";
        }
        ss << msec << " ms " << usec << " us";
        return ss.str();
    }
    static uint64_t measure_ticks_per_sec()
    {
        auto tm_start = timestamp();
        usleep(1000000);
        return timestamp() - tm_start;
    }
    static uint64_t timestamp() { return __rdtsc(); }
};
}
