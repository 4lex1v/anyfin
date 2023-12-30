
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/allocator.hpp"
#include "anyfin/core/strings.hpp"
#include "anyfin/core/result.hpp"

namespace Fin::Platform {

enum struct Platform {
  Win32,
};

struct System_Error {
  u32 error_code;

  Core::String retrieve_system_error_message (Core::Allocator_View allocator, const char **args, usize args_count);

  Core::String retrieve_system_error_message (Core::Allocator auto &allocator, Core::Convertible_To<const char *> auto&&... args) {
    const char *arg_array[] = { static_cast<const char *>(args)..., nullptr };
    return this->retrieve_system_error_message(allocator, arg_array, sizeof...(args));
  }
};

static inline Core::String to_string (const System_Error &error, Core::Allocator auto &allocator) {
  return String();
}

template <typename T>
using Result = Core::Result<System_Error, T>;

extern const Platform platform_type;

extern const char              platform_path_separator;
extern const Core::String_View platform_static_library_extension_name;
extern const Core::String_View platform_shared_library_extension_name;
extern const Core::String_View platform_executable_extension_name;
extern const Core::String_View platform_object_extension_name;

struct System_Command_Status {
  Core::String output;
  u32          status_code;
};

Result<System_Command_Status> run_system_command (const Core::String_View &command_line, Core::Allocator_View allocator);

u32 get_logical_cpu_count (); 

}

