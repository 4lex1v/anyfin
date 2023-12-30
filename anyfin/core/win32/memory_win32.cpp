
#include "anyfin/core/allocator.hpp"
#include "anyfin/core/memory.hpp"
#include "anyfin/core/strings.hpp"
#include "anyfin/core/trap.hpp"

#include "anyfin/core/win32/base_win32.hpp"

/*
  TODO: Implement memory functions using AVX instructions?
 */
extern "C" {

#pragma function(memset)
void * memset (void *destination, int value, size_t count) {
  auto storage = reinterpret_cast<u8 *>(destination);
  for (size_t idx = 0; idx < count; idx++) {
    storage[idx] = static_cast<u8>(value);
  }

  return destination;
}

#pragma function(memcpy)
void * memcpy (void *destination, const void *source, size_t count) {
  auto from = reinterpret_cast<const u8 *>(source);
  auto to   = reinterpret_cast<u8 *>(destination);
  for (size_t idx = 0; idx < count; idx++) {
    to[idx] = from[idx];
  }

  return destination;
}

}

namespace Fin::Core {

Memory_Region reserve_virtual_memory (usize size) {
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  
  auto aligned_size = align_forward(size, system_info.dwPageSize);

  auto memory = VirtualAlloc(0, aligned_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

  return Memory_Region { (u8 *) memory, aligned_size };
}

void release_virtual_memory (Memory_Region &memory) {
  VirtualFree(memory.memory, memory.size, MEM_RELEASE);
}

void enable_global_allocator_thread_safety () {
  trap("Global allocation thread safety is not supported at this point");
}

}
