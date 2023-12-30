
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/memory.hpp"
#include "anyfin/core/slice.hpp"
#include "anyfin/core/strings.hpp"

namespace Fin::Core {

/*
  Represents the CLI input argument, which is either an individual value, whatever it may be,
  or a pair, i.e a value in a form <key>=<value>.
 */
struct Startup_Argument {
  enum struct Type { Pair, Value };

  Type type;

  String_View key;
  String_View value;

  constexpr bool is_pair  () const { return type == Type::Pair; }
  constexpr bool is_value () const { return type == Type::Value; }
};

/*
  Main entry function defined by the application
*/
u32 app_entry (Slice<Startup_Argument>);

}

