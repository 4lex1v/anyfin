
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
  defer {  CloseHandle(child_stdout_read); };
  
  STARTUPINFO info {
    .cb         = sizeof(STARTUPINFO),
    .dwFlags    = STARTF_USESTDHANDLES,
    .hStdOutput = child_stdout_write,
    .hStdError  = child_stdout_write,
  };

  PROCESS_INFORMATION process {};
  if (!CreateProcess(nullptr, const_cast<char *>(command_line.value), &security, &security, TRUE, 0, NULL, NULL, &info, &process))
    return get_system_error();
  defer {
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
  };

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
      auto first_allocation_offset = static_cast<usize>(!output_buffer);
      auto reservation_size = bytes_available + first_allocation_offset;
      auto region = grow(allocator, &output_buffer, output_size, reservation_size, !!output_buffer, alignof(char));
      auto buffer = region - static_cast<usize>(!first_allocation_offset); // offset the added byte for subsequent allocations.

      DWORD bytes_read;
      if (!ReadFile(child_stdout_read, buffer, bytes_available, &bytes_read, NULL)) {
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

  /*
    While the above loop should ensure that the process has finished and exited (since we got the exit_code),
    something is still off and making subsquent calls to dependent files may fail, because the child process
    didn't release all resources. For example, in the test kit calling delete_directory to cleanup the testsite
    without this wait block, not all resources are released and the delete call may fail.
   */
  WaitForSingleObject(process.hProcess, INFINITE);

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
