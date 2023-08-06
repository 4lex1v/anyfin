
#pragma once

#include "anyfin/base.hpp"

struct Status_Code;

Status_Code create_window_system (const char *title);

bool pump_window_events ();
