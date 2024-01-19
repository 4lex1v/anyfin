
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/assert.hpp"
#include "anyfin/core/allocator.hpp"
#include "anyfin/core/memory.hpp"
#include "anyfin/core/meta.hpp"
#include "anyfin/core/prelude.hpp"
#include "anyfin/core/format.hpp"

namespace Fin::Core {

struct Memory_Arena {
  u8 *memory   = nullptr;
  usize size   = 0;
  usize offset = 0;

  constexpr Memory_Arena (u8 *_memory, const usize _size)
    : memory { _memory },
      size   { _size },
      offset { 0 }
  {
    assert(_memory);
    assert(size > sizeof(void*));
  }

  template <usize N>
  constexpr Memory_Arena (Byte_Array<N> auto (&array)[N])
    : Memory_Arena(cast_bytes<u8>(array), N) {}

  constexpr Memory_Arena (Memory_Region &&other)
    : Memory_Arena(other.memory, other.size)
  {
    other.memory = nullptr;
    other.size   = 0;
  }

  operator Allocator_View ();
};

static u8 * arena_reserve (Memory_Arena &arena, usize size, usize alignment, const Callsite_Info &callsite) {
  if (!size || !alignment) [[unlikely]] return nullptr;
  
  auto base         = arena.memory + arena.offset;
  auto aligned_base = align_forward(base, alignment);

  auto alignment_shift  = static_cast<usize>(aligned_base - base);
  auto reservation_size = alignment_shift + size;
  
  assert_caller((reservation_size + arena.offset) <= arena.size, callsite);
  if ((reservation_size + arena.offset) > arena.size) [[unlikely]] return nullptr;

  arena.offset += reservation_size;

  return aligned_base;
}

static u8 * arena_grow (Memory_Arena &arena, void *address, usize old_size, usize new_size,
                        bool immediate, usize reserve_alignment, const Callsite_Info &callsite) {
  if (address == nullptr) {
    if (old_size == 0) return arena_reserve(arena, new_size, reserve_alignment, callsite);
    return nullptr;
  }

  assert(address);
  assert(old_size);
  assert(new_size);

  if (immediate) [[likely]] {
    assert(arena.offset <= arena.size);

    const auto remaining_size = arena.size - arena.offset;
    if (remaining_size == 0) return nullptr;

    auto memory = arena.memory + arena.offset;
    arena.offset += new_size;

    return memory;
  }

  if (!address || !old_size || !new_size) [[unlikely]] return nullptr;

  const auto alignment = 1ULL << __builtin_ctzll(reinterpret_cast<usize>(address));
  auto new_region = arena_reserve(arena, new_size, alignment, callsite);
  if (!new_region) [[unlikely]] return nullptr;

  copy_memory(new_region, reinterpret_cast<u8 *>(address), old_size);

  return new_region;
}


static u8 * allocator_dispatch (Memory_Arena &arena, Allocation_Request request) {
  assert(arena.memory);
  assert(arena.size > 0);
  assert(arena.offset < arena.size);
  
  switch (request.type) {
    case Allocation_Request::Reserve: {
      assert(request.size >= 1);
      assert(request.alignment >= 1);

      return arena_reserve(arena, request.size, request.alignment, request.callsite);
    }
    case Allocation_Request::Grow: {
      assert(request.size >= 1);

      return arena_grow(arena, request.address, request.old_size, request.size, request.immediate, request.alignment, request.callsite);
    }
    case Allocation_Request::Free: {
      if (request.immediate) {
        auto edge = arena.memory + arena.offset;
        auto diff = usize(edge) - usize(request.address);
        assert(diff > 0);

        arena.offset -= diff;
      }

      return nullptr;
    }
  }
}

inline Memory_Arena::operator Allocator_View () {
  using Arena_Dispatcher = u8 * (*) (Memory_Arena &, Allocation_Request);
  const Arena_Dispatcher dispatcher = allocator_dispatch;

  Allocator_View view;
  view.value    = this;
  view.dispatch = reinterpret_cast<Allocator_View::Dispatch_Func>(dispatcher);

  return view;
}

static void reset_arena (Memory_Arena &arena) {
  arena.offset = 0;
}

static usize get_remaining_size (const Memory_Arena &arena) {
  return arena.size - arena.offset;
}

template <typename T = char>
static T * get_memory_at_current_offset (Memory_Arena &arena, const usize alignment = alignof(T)) {
  return reinterpret_cast<T *>(align_forward(arena.memory + arena.offset, alignment));
}

static Memory_Arena make_sub_arena (Memory_Arena &arena, const usize size, Callsite_Info callsite = {}) {
  auto reservation = reserve<u8>(arena, size, alignof(void *), callsite);
  assert(reservation != nullptr);

  return Memory_Arena(reservation, size);
}

template <usize Size>
struct Local_Arena {
  u8 reservation[Size];
  Memory_Arena arena;

  constexpr Local_Arena (): arena { reservation } {}

  operator Memory_Arena & () { return arena; }

  constexpr operator Allocator_View () { return arena; }
};

}
