
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/allocator.hpp"

namespace Fin::Core {

template <typename T>
struct Array {
  Allocator_View allocator;

  T     *values    = nullptr;
  usize  count     = 0;

  Array () = default;

  Array (T *_values, usize _count, Allocator auto &_allocator)
    : allocator { _allocator }, values { _values }, count { _count }
  {}

  ~Array () {
    free_memory(this->allocator, values);
  }

  const T& operator [] (usize offset) const { return values[offset]; }
  T&       operator [] (usize offset)       { return values[offset]; }
};

}
