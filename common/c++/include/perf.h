#pragma once
#include <immintrin.h>

namespace utils
{
uint64_t get_ticks() { return __rdtsc(); }

uint64_t calc_ticks_per_sec()
{
    auto tm_start = get_ticks();
    usleep(1000000);
    return get_ticks() - tm_start;
}
}
