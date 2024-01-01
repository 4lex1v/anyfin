
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


u32 get_logical_cpu_count () {
  SYSTEM_INFO systemInfo;
  GetSystemInfo(&systemInfo);

  return systemInfo.dwNumberOfProcessors;
}

}
