
#pragma once

#include "anyfin/base.hpp"

struct Status_Code;

struct System_Event;

Status_Code create_window_system (const char *title, usize width, usize height);

Status_Code present_frame ();

/*
  Reads buffered system events into a local buffer and returns that buffer to the caller via
  `system_events`, along with the number of events in `events_count`.

  In case the user attempts to close the window, `true` value is returned.
 */
bool pump_window_events (System_Event **system_events, usize *events_count);
