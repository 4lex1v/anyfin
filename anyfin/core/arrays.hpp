
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

  constexpr decltype(auto) operator [] (this auto &&self, usize offset) {
    return self.values[offset];
  }

  constexpr Slice<T> operator + (this auto &self, usize offset) {
    assert(offset <= self.count);
    return Slice(self.values + offset, self.count - offset);
  }

  constexpr decltype(auto) begin (this auto &&self) { return self.values; }
  constexpr decltype(auto) end   (this auto &&self) { return self.values + self.count; }

  fin_forceinline
  constexpr operator Slice<T> (this auto &&self) { return Slice(self.values, self.count); }
};

template <typename T>
fin_forceinline
static Slice<T> slice (Array<T> &array) {
  return Slice(array.values, array.count);
}

template <typename T>
static void destroy (Array<T> &array, Callsite_Info callsite = {}) {
  for (auto &elem: array) smart_destroy(elem, callsite);
  free(array.allocator, array.values);
}

template <typename T>
static Array<T> reserve_array (Allocator auto &allocator, const usize count, const usize alignment = alignof(T), const Callsite_Info callsite = {}) {
  assert_caller(count > 0, callsite);
  auto reservation = reserve<T>(allocator, count, alignment, callsite);
  return Array(allocator, reservation, count);
}

}
