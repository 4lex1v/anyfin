
#pragma once

#include "anyfin/base.hpp"

namespace Fin {

template <typename T>
struct Slice {
  T *values = nullptr;
  usize count = 0;

  constexpr Slice () = default;

  template <usize N>
  constexpr Slice (T (&data)[N])
    : Slice(data, N) {}
  
  constexpr Slice (T *_value, usize _count)
    : values { _value }, count { _count } {}

  constexpr operator bool (this auto self) { return self.values && self.count; }

  constexpr decltype(auto) operator [] (this auto &&self, usize offset) { return self.value[offset]; }
  constexpr decltype(auto) operator *  (this auto &&self)               { return *self.values; }

  constexpr Slice<T> operator + (this auto self, usize offset) {
    assert(offset <= self.count);
    return Slice(self.values + offset, self.count - offset);
  }

  constexpr Slice<T>& operator += (this Slice<T> &self, usize offset) {
    fin_ensure(offset <= self.count);

    self.values += offset;
    self.count  -= offset;

    return self;
  }

  constexpr Slice<T> operator ++ (this Slice<T> &self, int) { return (self += 1); }

  constexpr decltype(auto) begin (this auto self) { return self.values; }
  constexpr decltype(auto) end   (this auto self) { return self.values + self.count; }
};

template <typename T>
constexpr bool is_empty (Slice<T> args) {
  return args.count == 0;
}

}
