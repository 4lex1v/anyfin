
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/meta.hpp" // for the is_pointer check is align function

extern "C" {
void * memset (void *destination, int value, size_t count);
void * memcpy (void *destination, const void *source, size_t count);
}

namespace Fin::Core {

template <typename T>
constexpr T align_forward (const T value, const usize by) {
  if constexpr (!is_pointer<T>) return (value + (by - 1)) & ~(by - 1);
  else return reinterpret_cast<T>((reinterpret_cast<usize>(value) + (by - 1)) & ~(by - 1));  
}

constexpr auto align_forward_to_pow_2 (const auto value) {
  if (value == 0) return 1;

  const auto lead_zero_count = __builtin_clz(value);
  return 1 << ((sizeof(decltype(value)) * 8) - lead_zero_count);
}

constexpr bool is_aligned_by (const auto value, const usize by) {
  return value == align_forward(value, by);
}

template <typename T>
constexpr void copy_memory (T *destination, const T *source, const usize count) {
  __builtin_memcpy(destination, source, sizeof(T) * count);
}

template <typename T>
constexpr void zero_memory (T *memory, const usize count = 1) {
  __builtin_memset(memory, 0, sizeof(T) * count);
}

template <typename T>
constexpr bool compare_bytes (const T *a, const T *b, const usize count) {
  return (__builtin_memcmp(a, b, sizeof(T) * count) == 0);
}

static const char * get_character_offset_reversed (const char *memory, const char value, const usize length) {
  auto end = memory + length;

  auto cursor = memory;
  while (cursor < end) {
    if (*cursor == value) return cursor;
    cursor += 1;
  }

  return nullptr;
}

struct Memory_Region {
  u8    *memory;
  usize  size;
};

static Memory_Region reserve_virtual_memory (usize size);

static void free_virtual_memory (Memory_Region &region);

}

#ifndef MEMORY_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "anyfin/core/memory_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif

