
#pragma once

#include "anyfin/core/arrays.hpp"
#include "anyfin/core/strings.hpp"
#include "anyfin/core/option.hpp"

namespace Fin::Platform {

/*
  Represents the CLI input argument, which is either an individual value, whatever it may be,
  or a pair, i.e a value in a form <key>=<value>.
 */
struct Startup_Argument {
  enum struct Type { Pair, Value };

  Type type;

  Core::String_View key;
  Core::String_View value;

  constexpr bool is_pair  () const { return type == Type::Pair; }
  constexpr bool is_value () const { return type == Type::Value; }
};

constexpr Core::Option<Core::String_View> get_value (const Core::Iterable<Startup_Argument> auto &args, Core::String_View key_name) {
  for (auto &arg: args) {
    if (arg.is_value()) continue;
    if (compare_strings(arg.key, key_name)) return Core::String_View(arg.value);
  }

  return Core::opt_none;
}

static Core::Array<Startup_Argument> get_startup_args (Core::Allocator auto &allocator);

}

#ifndef STARTUP_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "startup_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
