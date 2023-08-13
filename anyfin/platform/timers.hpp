
#pragma once

#include "anyfin/base.hpp"

struct Status_Code;

Status_Code enable_high_precision_timer ();
void disable_high_precision_timer ();

u64 get_timer_frequency ();

u64 get_timer_value ();

u64 get_elapsed_millis (u64 frequency, u64 from, u64 to);
