
#define COMMANDS_HPP_IMPL

#include "anyfin/core/meta.hpp"
#include "anyfin/core/win32.hpp"

#include "anyfin/platform/commands.hpp"

namespace Fin::Platform {

static Result<System_Command_Status> run_system_command (Core::Allocator auto &allocator, Core::String_View command_line) {
  PROCESS_INFORMATION process  {};
  SECURITY_ATTRIBUTES security { .nLength = sizeof(SECURITY_ATTRIBUTES), .bInheritHandle = TRUE };

  HANDLE child_stdout_read, child_stdout_write;
  CreatePipe(&child_stdout_read, &child_stdout_write, &security, 0);
  SetHandleInformation(&child_stdout_read, HANDLE_FLAG_INHERIT, 0);

  STARTUPINFO info {
    .cb         = sizeof(STARTUPINFO),
    .dwFlags    = STARTF_USESTDHANDLES,
    .hStdOutput = child_stdout_write,
    .hStdError  = child_stdout_write,
  };

  if (!CreateProcess(nullptr, const_cast<char *>(command_line.value), &security, &security, TRUE, 0, NULL, NULL, &info, &process))
    return Core::Error(get_system_error());

  CloseHandle(child_stdout_write);

  char *output_buffer = nullptr;
  usize output_size   = 0;
  {
    enum { read_buffer_size = 1024 };
    char read_buffer[read_buffer_size];

    while (true) {
      DWORD bytes_read;
      if (!ReadFile(child_stdout_read, read_buffer, read_buffer_size, &bytes_read, NULL)) {
        if (output_buffer) free(allocator, output_buffer, true);
        return get_system_error();
      }

      if (!bytes_read) break;

      auto region = grow(allocator, &output_buffer, output_size, bytes_read, output_buffer != nullptr, alignof(char));
      Core::copy_memory(region, read_buffer, bytes_read);

      output_size += bytes_read;
    }
  }

  DWORD return_value = 0;
  GetExitCodeProcess(process.hProcess, &return_value);

  /*
    For some reason when WaitForSingleObject comes before reading from a pipe it may hang indefinitely,
    I'm not sure why this happens at this point. I'd like it to close the process first, before reading
    from the pipe, otherwise profiler shows that reading takes a lot of time. I'll investigate this later.
   */
  WaitForSingleObject(process.hProcess, INFINITE);
  CloseHandle(child_stdout_read);

  CloseHandle(process.hProcess);
  CloseHandle(process.hThread);

  if (!output_size) return Core::Ok(System_Command_Status { .status_code = return_value });

  return Core::Ok(System_Command_Status {
    .output      = Core::String(allocator, output_buffer, output_size),
    .status_code = return_value,
  });
}

}
