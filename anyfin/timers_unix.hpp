#define TIMERS_HPP_IMPL

// No need to include "anyfin/win32.hpp"

#include "anyfin/timers.hpp"

#include <time.h>
#include <unistd.h>

namespace Fin {

// static Sys_Result<Timer_Error, void> enable_high_precision_timer () {
//   // High-precision timers are enabled by default on Unix systems
//   return Ok();
// }

// static void disable_high_precision_timer () {
//   // No action needed
// }

static u64 get_timer_frequency () {
  return 1000000000ULL; // Nanoseconds per second
}

static u64 get_timer_value () {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<u64>(ts.tv_sec) * 1000000000ULL + ts.tv_nsec;
}

static u64 get_elapsed_millis (u64 frequency, u64 from, u64 to) {
  u64 elapsed = to - from;

  // Convert nanoseconds to milliseconds
  return elapsed / 1000000ULL;
}

}
