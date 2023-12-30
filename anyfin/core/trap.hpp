
#pragma once

#include "anyfin/base.hpp"

namespace Fin::Core {

typedef void (*Crash_Handler) (u32 exit_code);

void set_crash_handler (Crash_Handler handler);

/*
  Immediately terminate the executing process, returning the `exit_code` value to the system.
 */
[[noreturn]] void terminate (u32 exit_code);

/*
  Terminates the executing process with a provided error message.
 */
[[noreturn]] void trap (const char *message);

}
