
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/allocator.hpp"
#include "anyfin/core/strings.hpp"
#include "anyfin/core/result.hpp"

namespace Fin::Platform {

enum struct Platform {
  Win32,
};

static Platform get_platform_type ();
static bool is_win32 () { return get_platform_type() == Platform::Win32; }

struct System_Error {
  u32 error_code;
};

static Core::String retrieve_system_error_message (Core::Allocator auto &allocator, System_Error error, Core::Convertible_To<const char *> auto&&... args); 

static System_Error get_system_error ();

static auto to_string (System_Error error, Core::Allocator auto &allocator) {
  auto message = retrieve_system_error_message(allocator, error);
  return format_string(allocator, "System error: %, details: %", error.error_code, message);
}

template <typename T>
using Result = Core::Result<System_Error, T>;

static u32 get_logical_cpu_count (); 

}

#ifndef PLATFORM_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "platform_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
