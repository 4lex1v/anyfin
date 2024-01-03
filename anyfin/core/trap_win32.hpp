
#define TRAP_HPP_IMPL

#include "anyfin/core/trap.hpp"
#include "anyfin/core/win32.hpp"

namespace Fin::Core {

static Crash_Handler crash_handler;

static void set_crash_handler (Crash_Handler handler) {
  crash_handler = handler;
}

static void trap (const char *message) {
  auto message_length = 0;
  while (message[message_length++]);

  auto stdout = GetStdHandle(STD_OUTPUT_HANDLE);
  WriteConsole(stdout, message, message_length, nullptr, nullptr);
  
  crash_handler(1);
  __builtin_unreachable();
}

static void terminate (const u32 exit_code) {
  TerminateProcess(GetCurrentProcess(), exit_code);
  __builtin_unreachable();
}

}
