
#define COMMANDS_HPP_IMPL

#include "anyfin/core/meta.hpp"
#include "anyfin/core/win32.hpp"
#include "anyfin/core/assert.hpp"

#include "anyfin/platform/commands.hpp"

namespace Fin::Platform {

static Result<System_Command_Status> run_system_command (Core::Allocator auto &allocator, Core::String_View command_line) {
  SECURITY_ATTRIBUTES security { .nLength = sizeof(SECURITY_ATTRIBUTES), .bInheritHandle = TRUE };

  HANDLE child_stdout_read, child_stdout_write;
  if (!CreatePipe(&child_stdout_read, &child_stdout_write, &security, 0))   return get_system_error();
  if (!SetHandleInformation(child_stdout_read, HANDLE_FLAG_INHERIT, FALSE)) return get_system_error();
  
  STARTUPINFO info {
    .cb         = sizeof(STARTUPINFO),
    .dwFlags    = STARTF_USESTDHANDLES,
    .hStdOutput = child_stdout_write,
    .hStdError  = child_stdout_write,
  };

  PROCESS_INFORMATION process {};
  if (!CreateProcess(nullptr, const_cast<char *>(command_line.value), &security, &security, TRUE, 0, NULL, NULL, &info, &process))
    return get_system_error();

  CloseHandle(child_stdout_write);

  DWORD exit_code = 0;

  char *output_buffer = nullptr;
  usize output_size   = 0;
  {
    while (true) {
      DWORD bytes_available = 0;
      if (!PeekNamedPipe(child_stdout_read, NULL, 0, NULL, &bytes_available, NULL)) {
        auto error_code = get_system_error_code();
        if (error_code != ERROR_BROKEN_PIPE) {
          if (output_buffer) free(allocator, output_buffer, true);
          return get_system_error();
        }
      }

      if (bytes_available == 0) {
        GetExitCodeProcess(process.hProcess, &exit_code);
        if (exit_code != STILL_ACTIVE) break;
        continue;
      }

      /*
        Tiny hack to reserve space for the terminating 0
       */
      auto reservation_size = bytes_available + static_cast<usize>(!output_buffer);
      auto region = grow(allocator, &output_buffer, output_size, reservation_size, !!output_buffer, alignof(char));

      DWORD bytes_read;
      if (!ReadFile(child_stdout_read, region, bytes_available, &bytes_read, NULL)) {
        auto error_code = get_system_error_code();
        /*
          According to ReadFile docs if the child process has closed its end of the pipe, indicated
          by the BROKEN_PIPE status, we can treat that as EOF.
         */
        if (error_code != ERROR_BROKEN_PIPE) {
          if (output_buffer) free(allocator, output_buffer, true);
          return get_system_error();
        }
      }

      assert(bytes_read == bytes_available);

      output_size += bytes_read;
    }
  }

  CloseHandle(process.hProcess);
  CloseHandle(process.hThread);
  CloseHandle(child_stdout_read);

  if (!output_size) return Core::Ok(System_Command_Status { .status_code = exit_code });

  /*
    For some reason Windows includes CRLF at the end of the output, which is inc
   */
  if (output_size > 2 && Core::ends_with(output_buffer, "\r\n")) output_size -= 2;
  output_buffer[output_size] = '\0';

  return Core::Ok(System_Command_Status {
    .output      = Core::String(allocator, output_buffer, output_size),
    .status_code = exit_code,
  });
}

}
