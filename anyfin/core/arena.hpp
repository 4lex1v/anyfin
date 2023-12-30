
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

u8 * reserve_memory (Memory_Arena &arena, const usize size, const usize alignment = alignof(void *), const Callsite_Info info = Callsite_Info());

void free_reservation (Memory_Arena &arena, void *address, const Callsite_Info info = Callsite_Info());

u8 * grow_reservation (Memory_Arena &arena, void *address, const usize old_size, const usize new_size, const Callsite_Info info = Callsite_Info());

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

  ~Scope_Allocator () {}

  operator Allocator_View ();

  // u8 * reserve (usize size, usize alignment, const Callsite_Info info = Callsite_Info()) {
  //   return reserve_(size, alignment, info);
  // }

  // void free (void *address, const Callsite_Info info = Callsite_Info()) {
  //   arena.free(address, info);
  // }

  // u8 * grow (void *address, usize old_size, usize new_size, const Callsite_Info info = Callsite_Info()) {
  //   return arena.grow(address, old_size, new_size, info);
  // }
};

}
