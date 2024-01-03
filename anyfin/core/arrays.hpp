
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/allocator.hpp"
#include "anyfin/core/slice.hpp"

namespace Fin::Core {

template <typename T>
struct Array {
  Allocator_View allocator;

  T     *values    = nullptr;
  usize  count     = 0;

  constexpr Array () = default;

  constexpr Array (Allocator auto &_allocator, T *_values, usize _count)
    : allocator { _allocator }, values { _values }, count { _count }
  {}

  constexpr T &       operator [] (usize offset)       { return values[offset]; }
  constexpr T const & operator [] (usize offset) const { return values[offset]; }

  Slice<T> operator + (usize offset) const {
    assert(offset <= this->count);
    return Slice(this->values + offset, this->count - offset);
  }

  T *       begin ()       { return this->values; }
  T const * begin () const { return this->values; }

  T *       end ()       { return this->values + this->count; }
  T const * end () const { return this->values + this->count; }
};

template <typename T>
fin_forceinline
static Slice<T> slice (Array<T> &array) {
  return Slice(array.values, array.count);
}

template <typename T>
static void destroy (Array<T> &array) {
  free_memory(array.allocator, array.values);
}

template <typename T>
static Array<T> reserve_array (Allocator auto &allocator, const usize count, const usize alignment = alignof(T), const Callsite_Info info = {}) {
  auto reservation = reserve_memory<T>(allocator, count, alignment, info);
  return Array(allocator, reservation, count);
}

}
