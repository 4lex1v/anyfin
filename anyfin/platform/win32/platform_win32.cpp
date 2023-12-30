
#include "anyfin/core/list.hpp"

#include "anyfin/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include "anyfin/platform/win32/base_win32.hpp"

namespace Fin::Platform {

const char              platform_path_separator                = '\\';
const Core::String_View platform_static_library_extension_name = "lib";
const Core::String_View platform_shared_library_extension_name = "dll";
const Core::String_View platform_executable_extension_name     = "exe";
const Core::String_View platform_object_extension_name         = "obj";

Core::String System_Error::retrieve_system_error_message (const Core::Allocator &allocator, const char **arg_array, usize args_count) {
  const auto flags    = FORMAT_MESSAGE_FROM_SYSTEM | (args_count > 0 ? FORMAT_MESSAGE_ARGUMENT_ARRAY : FORMAT_MESSAGE_IGNORE_INSERTS);
  const auto language = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);

  const auto buffer_size = FormatMessageA(flags, nullptr, this->error_code, language, nullptr, 0, (va_list *)arg_array);

  auto buffer = reinterpret_cast<char *>(allocator.reserve(buffer_size));

  const auto message_size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | flags, nullptr, this->error_code, language, (LPSTR)&buffer, 0, (va_list *)arg_array);

  return Core::String(allocator, buffer, buffer_size);
}

Result<System_Command_Status> run_system_command (const Core::String_View &command_line, Core::Allocator &allocator) {
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

  if (CreateProcess(nullptr, const_cast<char *>(command_line.value), &security, &security, TRUE, 0, NULL, NULL, &info, &process) == false)
    return Core::Error(get_system_error());

  CloseHandle(child_stdout_write);

  Core::List<Core::String> chunks { allocator };
  usize total_output_size = 0;
  while (true) {
    enum { buffer_size = 1024 };
    auto buffer = reinterpret_cast<char *>(allocator.reserve(buffer_size, alignof(char)));

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

  if (total_output_size == 0) return Core::Ok(result);

  auto buffer = allocator.reserve(total_output_size + 1);
  for (auto offset = 0; auto &chunk: chunks) {
    memcpy_s(buffer + offset, total_output_size - offset, chunk.value, chunk.length);
    offset += chunk.length;
  }

  result.output = move(Core::String(allocator, reinterpret_cast<char *>(buffer), total_output_size));

  return Core::Ok(result);
}

u32 get_logical_cpu_count () {
  SYSTEM_INFO systemInfo;
  GetSystemInfo(&systemInfo);

  return systemInfo.dwNumberOfProcessors;
}

}
