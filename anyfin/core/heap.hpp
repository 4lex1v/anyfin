
#pragma once

#include "anyfin/base.hpp"

constexpr usize DEFAULT_ALIGNMENT = sizeof(void *);

struct Heap {
  struct Block;

  Memory_Region memory;
  
  Block *blocks      = nullptr;
  Block *free_blocks = nullptr;

  Heap () = default;
  Heap (Memory_Region region);
};
  
u8 *  heap_alloc   (Heap &heap, usize size, usize alignment, const char *tag);
u8 *  heap_realloc (Heap &heap, void *ptr, usize new_size, const char * tag);
void  heap_free    (Heap &heap, void *ptr, const char *tag);

template <typename T>
T & heap_push_struct (Heap &heap, const char *tag, usize alignment = alignof(T)) {
  return reinterpret_cast<T &>(heap_alloc(heap, sizeof(T), alignment, tag));
}

void report_leaks (const Heap &heap);
void destroy (Heap &heap);

