
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/assert.hpp"

namespace Fin::Core {

template <typename T>
struct Slice {
  T *elements = nullptr;
  usize count = 0;

  Slice () = default;

  template <usize N>
  Slice (T (&data)[N]): Slice(data, N) {}
  
  Slice (T *_elements, usize _count)
    : elements { _elements },
      count { _count }
  {}

  constexpr operator bool () const { return !count; }

  T &       operator [] (usize offset)       { return elements[offset]; }
  T const & operator [] (usize offset) const { return elements[offset]; }

  Slice<T> operator + (usize offset) const {
    assert(offset < count);
    return Slice(this->elements + offset, this->count - offset);
  }

  T &       operator * ()       { return *elements; }
  T const & operator * () const { return *elements; }

  Slice<T> operator ++ (int) {
    assert(count > 0);

    auto current = *this;

    elements += 1;
    count    -= 1;

    return current;
  }

  Slice<T>& operator += (usize offset) {
    assert(offset < count);

    this->elements += offset;
    this->count    -= offset;

    return *this;
  }

  T * begin () { return elements; }
  T * end   () { return elements + count; }

  T const * begin () const { return elements; }
  T const * end   () const { return elements + count; }
};

template <typename T>
static inline bool is_empty (const Slice<T> &args) {
  return args.count == 0;
}

}
