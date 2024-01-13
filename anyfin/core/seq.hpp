
#pragma once

#include "anyfin/base.hpp"
#include "anyfin/core/allocator.hpp"
#include "anyfin/core/slice.hpp"

namespace Fin::Core {

template <typename T>
struct Seq {
  using Value_Type = T;

  Allocator_View allocator;

  T     *data     = nullptr;
  usize  count    = 0;
  usize  capacity = 0;

  Seq (const Callsite_Info callsite = Callsite_Info())
    : Seq(get_global_allocator(), 16) {};

  Seq (Allocator auto &_allocator, const usize initial_capacity,
       const Callsite_Info callsite = Callsite_Info())
    : allocator { _allocator },
      capacity  { initial_capacity }
  {
    auto memory = reserve<T>(_allocator, initial_capacity, alignof(T), callsite);
    if (!memory) trap("Allocator has ran out of available memory");

    this->data = reinterpret_cast<T *>(memory);
  }

  ~Seq () {
    assert(this->allocator);

    for (usize idx = 0; idx < this->count; idx++) {
      this->data[idx].~Value_Type();
    }

    free(*this->allocator, this->data);

    this->data     = nullptr;
    this->count    = 0;
    this->capacity = 0;
  }

  T       & operator [] (usize offset)       { return data[offset]; }
  T const & operator [] (usize offset) const { return data[offset]; }

  void grow_if_needed () {
    if (this->count == this->capacity) {
      auto new_capacity = this->capacity * 2;
      auto new_memory = grow(this->allocator, this->data, new_capacity);
      if (!new_memory) trap();

      this->data     = new_memory;
      this->capacity = new_capacity;
    }
  }

  operator Slice<T> () const {
    return Slice(data, count);
  }
};

template <typename T>
static void seq_push (Seq<T> &seq, typename Seq<T>::Value_Type &&value) {
  seq.grow_if_needed();
  seq[seq.count++] = move(value);
}

template <typename T>
static void seq_push_copy (Seq<T> &seq, const typename Seq<T>::Value_Type &value) {
  seq.grow_if_needed();
  seq[seq.count++] = value;
}

}
