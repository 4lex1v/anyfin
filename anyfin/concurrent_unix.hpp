#define FIN_CONCURRENT_HPP_IMPL

#include "anyfin/math.hpp"
#include "anyfin/concurrent.hpp"

// No need to include "anyfin/win32.hpp"

#include <semaphore.h>
#include <errno.h>
#include <cstring>

namespace Fin {

struct Semaphore {
  sem_t handle;
};

static Sys_Result<Semaphore> create_semaphore (u32 count) {
  Semaphore semaphore;
  int result = sem_init(&semaphore.handle, 0, count);
  if (result != 0) {
    return Error(get_system_error());
  }
  return Ok(semaphore);
}

static Sys_Result<void> destroy (Semaphore &semaphore) {
  int result = sem_destroy(&semaphore.handle);
  if (result != 0) {
    return Error(get_system_error());
  }
  return Ok();
}

static Sys_Result<u32> increment_semaphore (Semaphore &semaphore, u32 increment_value) {
  for (u32 i = 0; i < increment_value; ++i) {
    if (sem_post(&semaphore.handle) != 0) {
      return Error(get_system_error());
    }
  }
  // POSIX semaphores do not provide previous value; return 0 or adjust as needed
  return Ok<u32>(0);
}

static Sys_Result<void> wait_for_semaphore_signal (const Semaphore &semaphore) {
  int result = sem_wait(const_cast<sem_t*>(&semaphore.handle));
  if (result != 0) {
    return Error(get_system_error());
  }
  return Ok();
}

}
