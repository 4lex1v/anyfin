
#pragma once

/*
  This is a workaround the issue clang is having with MS "source_location" header, there's something going on with __cpp_consteval
  macro definition, which makes source_location not work properly.
 */
// #ifndef __cpp_consteval
//   #define __cpp_consteval
// #endif

// #include <source_location>

#include "anyfin/base.hpp"

namespace Fin::Core {

struct Callsite_Info {
  u32         line;
  u32         column;
  const char *file;
  const char *function;

  consteval Callsite_Info (const u32 _l = __builtin_LINE(), const u32 _c = __builtin_COLUMN(),
                           const char *_fl = __builtin_FILE(), const char *_fn = __builtin_FUNCTION())
    : line { _l }, column { _c}, file { _fl }, function { _fn} {}
};

}
