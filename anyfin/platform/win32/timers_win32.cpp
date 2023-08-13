
#pragma comment(lib, "winmm")

#include "anyfin/platform/win32/common_win32.hpp"
#include "anyfin/platform/timers.hpp"
#include "anyfin/core/status_code.hpp"

#include <timeapi.h>

Status_Code enable_high_precision_timer () {
  auto status = timeBeginPeriod(1);
  if (status == TIMERR_NOCANDO)
    return { Status_Code::System_Error, "High precision timer is not supported" };

  return Status_Code::Success;
}

void disable_high_precision_timer () {
  timeEndPeriod(1);
}

u64 get_timer_frequency () {
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);

  return frequency.QuadPart;
}

u64 get_timer_value () {
  LARGE_INTEGER stamp;
  QueryPerformanceCounter(&stamp);

  return stamp.QuadPart;
}

u64 get_elapsed_millis (u64 frequency, u64 from, u64 to) {
  u64 elapsed = to - from;

  elapsed *= 1000;
  elapsed /= frequency;

  return elapsed;
}
