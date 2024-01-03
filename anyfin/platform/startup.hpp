
#pragma once

#include "anyfin/core/arrays.hpp"
#include "anyfin/core/strings.hpp"

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

static Core::Array<Startup_Argument> get_startup_args (Core::Allocator auto &allocator);

}

#ifndef STARTUP_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "startup_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
