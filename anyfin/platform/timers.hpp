
#pragma once

#include "anyfin/base.hpp"
#include "anyfin/core/result.hpp"

namespace Fin::Core {

struct Timer_Error {};

Result<Timer_Error, void> enable_high_precision_timer ();
void disable_high_precision_timer ();

u64 get_timer_frequency ();

u64 get_timer_value ();

u64 get_elapsed_millis (u64 frequency, u64 from, u64 to);

}
