
#define TIMERS_HPP_IMPL

#include "anyfin/core/win32.hpp"

#include "anyfin/platform/timers.hpp"

namespace Fin::Core {

// static Core::Result<Timer_Error, void> enable_high_precision_timer () {
//   auto status = timeBeginPeriod(1);
//   if (status == TIMERR_NOCANDO) return Error(Timer_Error{});
//   return Ok();
// }

// static void disable_high_precision_timer () {
//   timeEndPeriod(1);
// }

static u64 get_timer_frequency () {
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);

  return frequency.QuadPart;
}

static u64 get_timer_value () {
  LARGE_INTEGER stamp;
  QueryPerformanceCounter(&stamp);

  return stamp.QuadPart;
}

static u64 get_elapsed_millis (u64 frequency, u64 from, u64 to) {
  u64 elapsed = to - from;

  elapsed *= 1000;
  elapsed /= frequency;

  return elapsed;
}

}
