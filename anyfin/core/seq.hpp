
#pragma once

#include "anyfin/base.hpp"
#include "anyfin/core/allocator.hpp"
#include "anyfin/core/slice.hpp"

namespace Fin::Core {

template <typename T>
struct Seq {
  using Value_Type = T;

  Allocator_View allocator {};

  T     *data     = nullptr;
  usize  count    = 0;
  usize  capacity = 0;

  constexpr Seq () = default;

  constexpr Seq (Allocator auto &_allocator, usize initial_capacity, Callsite_Info callsite = {})
    : allocator { _allocator }, capacity { initial_capacity }
  {
    this->data = reserve<T>(_allocator, initial_capacity, alignof(T), callsite);
    if (!this->data) trap("Allocator has ran out of available memory\n");
  }

  constexpr decltype(auto) operator [] (this auto &&self, usize offset) {
    assert(offset < self.capacity);
    return self.data[offset];
  }

  constexpr void grow_if_needed (Callsite_Info callsite) {
    if (this->count == this->capacity) {
      auto old_size = sizeof(T) * this->capacity;
      auto new_size = old_size * 2;

      this->data = grow<T>(this->allocator, &this->data, old_size, new_size, false, alignof(T), callsite);
      if (!this->data) trap("Allocator has ran out of available memory\n");

      this->capacity = this->capacity * 2;
    }
  }

  constexpr operator Slice<T> () const {
    return Slice<T>(data, count);
  }
};

template <typename T>
constexpr void destroy (Seq<T> &seq, Callsite_Info callsite = {}) {
  if (is_empty(seq)) return;

  for (auto &value: seq) smart_destroy(value, callsite);

  free(seq.allocator, seq.data, callsite);

  seq.data     = nullptr;
  seq.count    = 0;
  seq.capacity = 0;
}

template <typename T>
static void seq_push (Seq<T> &seq, typename Seq<T>::Value_Type &&value, Callsite_Info callsite = {}) {
  seq.grow_if_needed(callsite);
  seq[seq.count] = move(value);
  seq.count += 1;
}

template <typename T>
static void seq_push_copy (const Seq<T> &seq, const typename Seq<T>::Value_Type &value, Callsite_Info callsite = {}) {
  seq.grow_if_needed(callsite);
  seq[seq.count] = value;
  seq.count += 1;
}

template <typename T>
static Seq<T> reserve_seq (Allocator auto &allocator, usize count, usize alignment = alignof(T), Callsite_Info callsite = {}) {
  if (count == 0) return {};
  return Seq<T>(allocator, count, callsite);
}

}
