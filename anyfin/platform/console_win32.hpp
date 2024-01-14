
#define CONSOLE_HPP_IMPL

#include "anyfin/platform/console.hpp"
#include "anyfin/core/win32.hpp"

namespace Fin::Core {

static Fin::Platform::Result<void> print (String_View message) {
  if (!Win32_Internals::stdout_print(message, message.length)) {
    return Platform::get_system_error();
  }

  return Core::Ok();
}

// Status_Code attach_console () {
//   if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
//     if (!AllocConsole()) return get_system_error();

//     SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT);
//   }
//   return Status_Code::Success;
// }

// Status_Code send_bytes_to_stdout (const Memory_Region &buffer) {
//   auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
//   if (handle == INVALID_HANDLE_VALUE) return get_system_error();

//   DWORD bytes_written = 0;
//   if (!WriteFile(handle, buffer.memory, buffer.size, &bytes_written, nullptr))
//     return get_system_error();

//   return Status_Code::Success;
// }

// get_command_line()

}
