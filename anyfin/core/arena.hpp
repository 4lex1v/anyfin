
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/assert.hpp"
#include "anyfin/core/allocator.hpp"
#include "anyfin/core/meta.hpp"

namespace Fin::Core {

struct Memory_Arena {
  u8 * const memory;
  const usize size;
  usize offset;

  Memory_Arena () = delete;

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

static u8 * reserve_memory (Memory_Arena &arena, const usize size, const usize alignment = alignof(void *),
                            const Callsite_Info info = Callsite_Info()) {
  assert(is_power_of_2(alignment));
  assert(size > 0);
  assert(alignment > 0);
  
  if (!size || !alignment) [[unlikely]] return nullptr;
  
  auto base         = arena.memory + arena.offset;
  auto aligned_base = align_forward(base, alignment);

  auto alignment_shift  = static_cast<usize>(aligned_base - base);
  auto reservation_size = alignment_shift + size;
  
  if ((reservation_size + arena.offset) > arena.size) [[unlikely]] return nullptr;

  arena.offset += reservation_size;

  return aligned_base;
}

static void free_reservation (Memory_Arena &arena, void *address, const Callsite_Info info = Callsite_Info()) {}

static u8 * grow_reservation (Memory_Arena &arena, void *address, const usize old_size, const usize new_size,
                              const Callsite_Info info = Callsite_Info()) {
  if (!address || !old_size || !new_size) [[unlikely]] return nullptr;

  const auto alignment = 1ULL << __builtin_ctzll(reinterpret_cast<usize>(address));

  auto new_region = reserve_memory(arena, new_size, alignment, info);
  if (!new_region) [[unlikely]] return nullptr;

  memcpy_s(new_region, new_size, address, old_size);

  return new_region;
}

inline Memory_Arena::operator Allocator_View () {
  using Arena_Reserve_Func = u8 * (*) (Memory_Arena &, usize, usize, Callsite_Info); 
  using Arena_Free_Func    = void (*) (Memory_Arena &, void *, Callsite_Info);
  using Arena_Grow_Func    = u8 * (*) (Memory_Arena &, void *, usize, usize, Callsite_Info);

  Arena_Reserve_Func arena_reserve = reserve_memory;
  Arena_Free_Func    arena_free    = free_reservation;
  Arena_Grow_Func    arena_grow    = grow_reservation;

  Allocator_View view;
  view.underlying = this;
  view.reserve    = reinterpret_cast<Allocator_View::Reserve_Func>(arena_reserve);
  view.grow       = reinterpret_cast<Allocator_View::Grow_Func>(arena_grow);
  view.free       = reinterpret_cast<Allocator_View::Free_Func>(arena_free);

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
  return Memory_Arena(reserve_memory(arena, size, alignof(void *)), size);
}

template <typename T, usize Align = alignof(T), typename... Args>
static T * reserve_struct (Memory_Arena &arena, Args&&... args) {
  auto memory = reserve_memory(arena, sizeof(T), Align);
  if (!memory) [[unlikely]] return nullptr;

  return new (memory) T { forward<Args>(args)... };
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

}
