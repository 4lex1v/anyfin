
#define PLATFORM_HPP_IMPL

#include "anyfin/core/assert.hpp"
#include "anyfin/core/strings.hpp"
#include "anyfin/core/win32.hpp"

#include "anyfin/platform/platform.hpp"

namespace Fin::Platform {

static Platform get_platform_type () { return Platform::Win32; }

static u32 get_system_error_code () {
  return GetLastError();
}

static System_Error get_system_error (Core::Convertible_To<const char *> auto&&... args) {
  auto error_code = get_system_error_code();
  
  const auto flags    = FORMAT_MESSAGE_FROM_SYSTEM | (sizeof...(args) > 0 ? FORMAT_MESSAGE_ARGUMENT_ARRAY : FORMAT_MESSAGE_IGNORE_INSERTS);
  const auto language = MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);

  const char *arg_array[] = { args..., nullptr };

  LPSTR message = nullptr;
  FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | (sizeof...(args) > 0 ? FORMAT_MESSAGE_ARGUMENT_ARRAY : FORMAT_MESSAGE_IGNORE_INSERTS),
    nullptr, error_code, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (LPSTR)&message, 0, (va_list *)arg_array);

  return System_Error { error_code, Core::String_View(message) };
}

static void destroy (System_Error error) {
  LocalFree((HLOCAL)error.details.value);
}

static u32 get_logical_cpu_count () {
  SYSTEM_INFO systemInfo;
  GetSystemInfo(&systemInfo);

  return systemInfo.dwNumberOfProcessors;
}

static Result<Core::Option<Core::String>> get_env_var (Core::Allocator auto &allocator, Core::String_View name) {
  auto reservation_size = GetEnvironmentVariable(name, nullptr, 0);
  if (!reservation_size) return get_system_error();

  auto env_value_buffer = reserve(allocator, reservation_size);
  auto env_value_length = GetEnvironmentVariable(name, env_value_buffer, reservation_size);
  if (!env_value_length) return get_system_error();

  assert(env_value_length == (reservation_size - 1));

  return Core::Option(Core::String(allocator, env_value_buffer, env_value_length));
}

}
