#ifndef CHAMPSIM_H
#define CHAMPSIM_H

#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <iomanip>

#include "champsim_constants.h"

// USEFUL MACROS
//#define DEBUG_PRINT
//#define DEBUG_PRINT_V2
#define SANITY_CHECK
#define LLC_BYPASS
#define DRC_BYPASS
#define NO_CRC2_COMPILE

#define ENABLE_EXTRA_CACHE_STATS
//#define ALWAYS_HIT
//#define ALWAYS_MISS
//#define NO_IMISS
//#ifdef MULTIPLE_PAGE_SIZE
//#define BASE_PAGE_SIZE 4096
//#define LOG2_BASE_PAGE_SIZE lg2(BASE_PAGE_SIZE)
//#define LARGE_PAGE_SIZE 2097152
//#define LOG2_LARGE_PAGE_SIZE lg2(LARGE_PAGE_SIZE)
//#endif

#ifdef DEBUG_PRINT
#define DP(x) x
#else
#define DP(x)
#endif
#define _DP(x)

#ifdef DEBUG_PRINT_V2
#define DP2(x) x
#else
#define DP2(x)
#endif

// CACHE
#define INFLIGHT 1
#define COMPLETED 2

#define FILL_L1 1
#define FILL_L2 2
#define FILL_LLC 4
#define FILL_DRC 8
#define FILL_DRAM 16

using namespace std;

extern uint8_t warmup_complete[NUM_CPUS];

namespace champsim
{
struct deadlock : public std::exception {
  const uint32_t which;
  explicit deadlock(uint32_t cpu) : which(cpu) {}
};

struct deprecated_clock_cycle {
  uint64_t operator[](std::size_t cpu_idx);
};
} // namespace champsim

extern champsim::deprecated_clock_cycle current_core_cycle;

#endif
