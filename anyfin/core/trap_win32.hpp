
#define TRAP_HPP_IMPL

#include "anyfin/core/trap.hpp"
#include "anyfin/core/win32.hpp"

namespace Fin::Core {

static void set_crash_handler (Crash_Handler handler) {
  crash_handler = handler;
}

static void trap (const char *message, const usize message_length) {
#ifdef DEV_BUILD
  if (IsDebuggerPresent()) __builtin_debugtrap();
#endif

  Win32_Internals::stdout_print(message, message_length);
  
  crash_handler(1);
  __builtin_unreachable();
}

static void trap (const char *message) {
  auto message_length = 0;
  while (message[message_length])
    message_length += 1;

  trap(message, message_length);
}

static void terminate (const u32 exit_code) {
  TerminateProcess(GetCurrentProcess(), exit_code);
  __builtin_unreachable();
}

}
