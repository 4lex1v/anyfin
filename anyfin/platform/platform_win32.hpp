
#define PLATFORM_HPP_IMPL

#include "anyfin/core/list.hpp"
#include "anyfin/core/win32.hpp"

#include "anyfin/platform/platform.hpp"

namespace Fin::Platform {

static Platform get_platform_type () { return Platform::Win32; }

static System_Error get_system_error () {
  return { GetLastError() };
}

static Core::String retrieve_system_error_message (Core::Allocator auto &allocator, System_Error error, Core::Convertible_To<const char *> auto&&... args) {
  const auto flags    = FORMAT_MESSAGE_FROM_SYSTEM | (sizeof...(args) > 0 ? FORMAT_MESSAGE_ARGUMENT_ARRAY : FORMAT_MESSAGE_IGNORE_INSERTS);
  const auto language = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);

  const char *arg_array[] = { args..., nullptr };

  const auto buffer_size = FormatMessageA(flags, nullptr, error.error_code, language, nullptr, 0, (va_list *)arg_array);

  auto buffer = reserve(allocator, buffer_size);

  const auto message_size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | flags, nullptr, error.error_code, language, (LPSTR)&buffer, 0, (va_list *)arg_array);

  return Core::String(allocator, buffer, buffer_size);
}

static u32 get_logical_cpu_count () {
  SYSTEM_INFO systemInfo;
  GetSystemInfo(&systemInfo);

  return systemInfo.dwNumberOfProcessors;
}

}
