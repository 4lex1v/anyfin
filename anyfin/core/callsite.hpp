
#pragma once

#include "anyfin/base.hpp"

namespace Fin::Core {

struct Callsite_Info {
  u32         line;
  u32         column;
  const char *file;
  const char *function;

  consteval Callsite_Info (const u32 _l = __builtin_LINE(), const u32 _c = __builtin_COLUMN(),
                           const char *_fl = __builtin_FILE(), const char *_fn = __builtin_FUNCTION())
    : line { _l }, column { _c }, file { _fl }, function { _fn }
  {}
};

static auto to_string (const Callsite_Info &callsite, auto &allocator) {
  return concat_string(allocator, callsite.file, "(", callsite.line, "):", callsite.function);
}

}
