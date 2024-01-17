
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/core.hpp"

namespace Fin::Core {

using Crash_Handler = void (*) (u32 crash_handler);

FIN_EXTERN_STATE Crash_Handler crash_handler;

static void set_crash_handler (Crash_Handler handler);

/*
  Immediately terminate the executing process, returning the `exit_code` value to the system.
 */
[[noreturn]] static void terminate (u32 exit_code);

/*
  Terminates the executing process with a provided error message.
 */
[[noreturn]] static void trap (const char *message);

/*
  Terminates the executing process with a provided error message.
 */
[[noreturn]] static void trap (const char *message, const usize message_length);

}

#ifndef TRAP_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "anyfin/core/trap_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
