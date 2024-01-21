
#define CONCURRENT_HPP_IMPL

#include "anyfin/core/math.hpp"
#include "anyfin/platform/concurrent.hpp"

#include "anyfin/core/win32.hpp"

namespace Fin::Platform {

static Result<Semaphore> create_semaphore (u32 count) {
  auto clamped = Core::clamp<s32>(count, 1, ~0x80000000);

  auto handle = CreateSemaphore(nullptr, 0, clamped, nullptr);
  if (!handle) return Core::Error(get_system_error());
  
  return Core::Ok(Semaphore { reinterpret_cast<Semaphore::Handle *>(handle) });
}

static Result<void> destroy (Semaphore &semaphore) {
  if (!CloseHandle(semaphore.handle))
    return Core::Error(get_system_error());

  semaphore.handle = nullptr;

  return Core::Ok();
}

static Result<u32> increment_semaphore (Semaphore &semaphore, u32 increment_value) {
  LONG previous;
  if (!ReleaseSemaphore(semaphore.handle, increment_value, &previous))
    return Core::Error(get_system_error());

  return Core::Ok<u32>(previous);
}

static Result<void> wait_for_semaphore_signal (const Semaphore &semaphore) {
  if (WaitForSingleObject(semaphore.handle, INFINITE) == WAIT_FAILED)
    return Core::Error(get_system_error());

  return Core::Ok();
}

}
