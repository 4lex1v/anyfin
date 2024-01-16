
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/assert.hpp"

namespace Fin::Core {

template <typename T>
struct Slice {
  T *value = nullptr;
  usize count = 0;

  constexpr Slice () = default;

  template <usize N>
  fin_forceinline
  constexpr Slice (T (&data)[N])
    : Slice(data, N) {}
  
  fin_forceinline
  constexpr Slice (T *_value, usize _count)
    : value { _value }, count { _count } {}

  fin_forceinline
  constexpr operator bool (this auto self) { return self.value && self.count; }

  fin_forceinline
  constexpr decltype(auto) operator [] (this auto &&self, usize offset) { return self.value[offset]; }

  fin_forceinline
  constexpr decltype(auto) operator *  (this auto &&self)               { return *self.value; }

  constexpr Slice<T> operator + (this auto self, usize offset) {
    assert(offset <= self.count);
    return Slice(self.value + offset, self.count - offset);
  }

  constexpr Slice<T>& operator += (this Slice<T> &self, usize offset) {
    assert(offset <= self.count);

    self.value += offset;
    self.count    -= offset;

    return self;
  }

  fin_forceinline
  constexpr Slice<T> operator ++ (this Slice<T> &self, int) { return (self += 1); }

  constexpr decltype(auto) begin (this auto self) { return self.value; }
  constexpr decltype(auto) end   (this auto self) { return self.value + self.count; }
};

template <typename T>
constexpr bool is_empty (Slice<T> args) {
  return args.count == 0;
}

namespace iterator {

template <typename T>
constexpr usize count (const Slice<T> &slice) {
  return slice.count;
}

}

}
