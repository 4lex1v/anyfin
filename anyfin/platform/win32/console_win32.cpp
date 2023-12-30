
#include "anyfin/core/strings.hpp"

#include "anyfin/core/win32/base_win32.hpp"

namespace Fin::Core {

/*
  TODO: Thread UNSAFE implementation
 */
void console_print_message (const String_View &message) {
  auto stdout = GetStdHandle(STD_OUTPUT_HANDLE);
  WriteConsole(stdout, message.value, message.length, nullptr, nullptr);
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
