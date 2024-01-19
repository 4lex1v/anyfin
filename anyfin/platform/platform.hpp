
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/allocator.hpp"
#include "anyfin/core/strings.hpp"
#include "anyfin/core/string_builder.hpp"
#include "anyfin/core/result.hpp"

namespace Fin::Platform {

enum struct Platform {
  Win32,
};

static Platform get_platform_type ();
static bool is_win32 () { return get_platform_type() == Platform::Win32; }

struct System_Error {
  u32 error_code;
  Core::String_View details;
};

static void destroy (System_Error error);

static u32 get_system_error_code ();

static System_Error get_system_error (Core::Convertible_To<const char *> auto&&... args);

template <Core::Allocator A>
static auto to_string (System_Error error, A &allocator) {
  Core::String_Builder builder { allocator };
  defer { alloc_destroy<A>(builder); };

  builder += "System code: ";
  builder += to_string(error.error_code, allocator);

  if (!is_empty(error.details)) {
    builder += " - ";
    builder += error.details;
  }

  return build_string(allocator, builder);
}

template <typename T>
using Result = Core::Result<System_Error, T>;

static u32 get_logical_cpu_count (); 

static Result<Core::Option<Core::String>> get_env_var (Core::Allocator auto &allocator, Core::String_View name);

}

#ifndef PLATFORM_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "platform_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
