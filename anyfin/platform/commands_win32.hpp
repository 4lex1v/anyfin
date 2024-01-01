
#ifdef COMMANDS_HPP_IMPL
  #error "This is wrong"
#endif

#include "anyfin/core/meta.hpp"

#define COMMANDS_HPP_IMPL
#include "anyfin/platform/commands.hpp"

#include "anyfin/platform/win32/base_win32.hpp"

namespace Fin::Platform {

static Result<System_Command_Status> run_system_command (Core::Allocator auto &allocator, const Core::String_View &command_line) {
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

  Core::List<Core::String> chunks { allocator };
  usize total_output_size = 0;
  while (true) {
    enum { buffer_size = 1024 };
    auto buffer = reinterpret_cast<char *>(reserve_memory(allocator, buffer_size, alignof(char)));

    DWORD bytes_read;
    ReadFile(child_stdout_read, buffer, buffer_size, &bytes_read, NULL);

    total_output_size += bytes_read;

    list_push(chunks, Core::String(allocator, buffer, bytes_read));
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

  System_Command_Status result { .status_code = return_value };

  if (!total_output_size) return Core::Ok(Core::move(result));

  auto buffer = reserve_memory(allocator, total_output_size + 1);
  for (auto offset = 0; auto &chunk: chunks) {
    memcpy_s(buffer + offset, total_output_size - offset, chunk.value, chunk.length);
    offset += chunk.length;
  }

  result.output = Core::move(Core::String(allocator, reinterpret_cast<char *>(buffer), total_output_size));

  return Core::Ok(Core::move(result));
}

}
