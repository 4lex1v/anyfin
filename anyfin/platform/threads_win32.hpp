
#define THREADS_HPP_IMPL

#include "anyfin/platform/threads.hpp"

namespace Fin::Platform {

template <typename T>
static Result<Thread> spawn_thread (const Core::Invocable<void, T *> auto &proc, T *data) {
  DWORD thread_id;
  auto handle = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(proc), data, 0, &thread_id);
  if (!handle) return Error(get_system_error());

  return Ok(Thread { reinterpret_cast<Thread::Handle *>(handle), thread_id });
}

static Result<Thread> spawn_thread (const Core::Invocable<void> auto &proc) {
  return spawn_thread(nullptr, proc);
}

static Result<void> shutdown_thread (Thread &thread);

static u32 get_current_thread_id () {
  return GetCurrentThreadId();
}

}
