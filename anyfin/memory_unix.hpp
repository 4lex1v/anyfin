#define FIN_MEMORY_HPP_IMPL

// No need to include "anyfin/win32.hpp"

#include "anyfin/memory.hpp"

#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

namespace Fin {

static Memory_Region reserve_virtual_memory (usize size) {
  long page_size = sysconf(_SC_PAGESIZE);
  const auto aligned_size = align_forward(size, page_size);

  void *memory = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (memory == MAP_FAILED) {
    return Memory_Region { nullptr, 0 };
  }

  return Memory_Region { reinterpret_cast<u8 *>(memory), aligned_size };
}

static void free_virtual_memory (Memory_Region &memory) {
  if (memory.memory && memory.size > 0) {
    munmap(memory.memory, memory.size);
  }
}

}
