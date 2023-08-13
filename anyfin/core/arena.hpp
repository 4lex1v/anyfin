
#pragma once

#include "anyfin/base.hpp"
#include "anyfin/core/prelude.hpp"

struct Memory_Arena {
  u8    *memory;
  usize  size;
  usize  offset;

  constexpr Memory_Arena (u8 *_memory, usize _size)
    : memory { _memory },
      size   { _size   },
      offset { 0 }
  {}

  constexpr Memory_Arena (Memory_Region region)
    : memory { region.memory },
      size   { region.size   },
      offset { 0 }
  {}
};

constexpr void reset_arena (Memory_Arena &arena) {
  arena.offset = 0;
}

template <typename T = char>
constexpr T * get_memory_at_current_offset (Memory_Arena &arena, usize alignment = alignof(T)) {
  return reinterpret_cast<T *>(align_forward(arena.memory + arena.offset, alignment));
}

constexpr u8 * reserve_memory (Memory_Arena &arena, usize size, usize alignment = alignof(void *)) {
  auto base         = arena.memory + arena.offset;
  auto aligned_base = align_forward(base, alignment);

  auto alignment_shift  = static_cast<usize>(aligned_base - base);
  auto reservation_size = alignment_shift + size;
  
  if ((reservation_size + arena.offset) > arena.size) return nullptr;

  arena.offset += reservation_size;

  return aligned_base;
}

constexpr u8 * reserve_memory_unsafe (Memory_Arena &arena, usize size, usize alignment = alignof(void *)) {
  auto base         = arena.memory + arena.offset;
  auto aligned_base = align_forward(base, alignment);

  auto alignment_shift  = static_cast<usize>(aligned_base - base);
  auto reservation_size = alignment_shift + size;

  arena.offset += reservation_size;

  return aligned_base;
}

template <typename T>
constexpr T * reserve_struct (Memory_Arena &arena, usize alignment = alignof(T)) {
  return reinterpret_cast<T *>(reserve_memory(arena, sizeof(T), alignment));
}

template <typename T = char>
constexpr T * reserve_array  (Memory_Arena &arena, usize count, usize alignment = alignof(T)) {
  if (count == 0) return nullptr;
  return reinterpret_cast<T *>(reserve_memory(arena, sizeof(T) * count, alignment));
}

template <typename T = char>
constexpr T * reserve_array_unsafe (Memory_Arena &arena, usize count, usize alignment = alignof(T)) {
  return reinterpret_cast<T *>(reserve_memory_unsafe(arena, sizeof(T) * count, alignment));
}
