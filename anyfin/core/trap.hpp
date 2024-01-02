
#pragma once

#include "anyfin/base.hpp"

namespace Fin::Core {

typedef void (*Crash_Handler) (u32 exit_code);

static void set_crash_handler (Crash_Handler handler);

/*
  Immediately terminate the executing process, returning the `exit_code` value to the system.
 */
[[noreturn]] static void terminate (u32 exit_code);

/*
  Terminates the executing process with a provided error message.
 */
[[noreturn]] static void trap (const char *message);

}

#ifndef TRAP_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "anyfin/core/trap_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
