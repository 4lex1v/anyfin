
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/assert.hpp"
#include "anyfin/core/allocator.hpp"
#include "anyfin/core/memory.hpp"
#include "anyfin/core/meta.hpp"
#include "anyfin/core/prelude.hpp"

namespace Fin::Core {

struct Memory_Arena {
  u8 * const memory;
  const usize size;
  usize offset;

  Memory_Arena (u8 *_memory, const usize _size)
    : memory { _memory },
      size   { _size }
  {}

  Memory_Arena (Memory_Region &&other)
    : memory { other.memory },
      size   { other.size },
      offset { 0 }
  {
    other.memory = nullptr;
    other.size   = 0;
  }

  operator Allocator_View ();
};

static u8 * allocator_dispatch (Memory_Arena &arena, const Allocation_Request request) {
  switch (request.type) {
    case Allocation_Request::Reserve: {
      assert(is_power_of_2(request.alignment));
      assert(request.size > 0);
      assert(request.alignment > 0);
  
      if (!request.size || !request.alignment) [[unlikely]] return nullptr;
  
      auto base         = arena.memory + arena.offset;
      auto aligned_base = align_forward(base, request.alignment);

      auto alignment_shift  = static_cast<usize>(aligned_base - base);
      auto reservation_size = alignment_shift + request.size;
  
      if ((reservation_size + arena.offset) > arena.size) [[unlikely]] return nullptr;

      arena.offset += reservation_size;

      return aligned_base;
    }
    case Allocation_Request::Grow: {
      if (!request.address || !request.old_size || !request.size) [[unlikely]] return nullptr;

      const auto alignment = 1ULL << __builtin_ctzll(reinterpret_cast<usize>(request.address));

      auto new_region = allocator_dispatch(arena, Allocation_Request(request.size, request.alignment, request.info));
      if (!new_region) [[unlikely]] return nullptr;

      copy_memory(new_region, (u8*) request.address, request.old_size);

      return new_region;
    }
    case Allocation_Request::Free: {
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

static Memory_Arena make_sub_arena (Memory_Arena &arena, const usize size) {
  return Memory_Arena(reserve_memory<u8>(arena, size, alignof(void *)), size);
}

template <>
struct Scope_Allocator<Memory_Arena> {
  Memory_Arena arena;

  Scope_Allocator (Memory_Arena &_arena)
    : arena  { _arena } {}

  operator Allocator_View ();
};

template <> void destroy (Scope_Allocator<Memory_Arena> &allocator) {
  
}

static u8 * allocator_dispatch (Scope_Allocator<Memory_Arena> &allocator, const Allocation_Request request) {
  return allocator_dispatch(allocator.arena, request);
}

inline Scope_Allocator<Memory_Arena>::operator Allocator_View () {
  using Dispatcher = u8 * (*) (Scope_Allocator<Memory_Arena> &, Allocation_Request);
  const Dispatcher dispatcher = allocator_dispatch;

  Allocator_View view;
  view.value    = this;
  view.dispatch = reinterpret_cast<Allocator_View::Dispatch_Func>(dispatcher);

  return view;
}

template <usize Size>
struct Local_Arena {
  u8 reservation[Size];
  Memory_Arena arena;

  constexpr Local_Arena (): arena { reservation, Size } {}

  operator Memory_Arena & () { return arena; }
};

template <usize Size>
static u8 * allocator_dispatch (Local_Arena<Size> &allocator, const Allocation_Request request) {
  return allocator_dispatch(allocator.arena, request);
}

}
