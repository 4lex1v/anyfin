
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/assert.hpp"

namespace Fin::Core {

template <typename T>
struct Slice {
  T *elements = nullptr;
  usize count = 0;

  constexpr Slice () = default;

  template <usize N>
  constexpr Slice (T (&data)[N])
    : Slice(data, N) {}
  
  constexpr Slice (T *_elements, usize _count)
    : elements { _elements }, count { _count } {}

  constexpr operator bool (this auto self) { return self.elements && self.count; }

  constexpr decltype(auto) operator [] (this auto &&self, usize offset) { return self.elements[offset]; }
  constexpr decltype(auto) operator *  (this auto &&self)               { return *self.elements; }

  constexpr Slice<T> operator + (this auto self, usize offset) {
    assert(offset <= self.count);
    return Slice(self.elements + offset, self.count - offset);
  }

  constexpr Slice<T>& operator += (this Slice<T> &self, usize offset) {
    assert(offset <= self.count);

    self.elements += offset;
    self.count    -= offset;

    return self;
  }

  constexpr Slice<T> operator ++ (this Slice<T> &self, int) { return (self += 1); }

  constexpr decltype(auto) begin (this auto self) { return self.elements; }
  constexpr decltype(auto) end   (this auto self) { return self.elements + self.count; }
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
