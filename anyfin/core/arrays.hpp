
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/allocator.hpp"

namespace Fin::Core {

template <typename T>
struct Array {
  Allocator allocator;

  T     *values    = nullptr;
  usize  count     = 0;

  Array () = default;

  Array (T *_values, usize _count, Allocator _allocator)
    : allocator { move(_allocator) }, values { _values }, count { _count }
  {}

  ~Array () {
    free_memory(this->allocator, values);
  }

  const T& operator [] (usize offset) const { return values[offset]; }
  T&       operator [] (usize offset)       { return values[offset]; }
};

template <typename T = char>
static auto reserve_array (Can_Reserve_Memory auto &allocator, const usize element_count, const usize alignment = alignof(T),
                           const Callsite_Info callsite = Callsite_Info::current()) {
  using Alloc_Type = raw_type<decltype(allocator)>;
  return Array<T>(allocator.reserve(sizeof(T) * element_count, alignment, callsite), element_count, allocator);
}

}
