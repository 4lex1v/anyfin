
#include "anyfin/platform/threads.hpp"

namespace Fin::Platform {

Result<Thread> spawn_thread (const Closure<u32 ()> &proc) {
  DWORD thread_id;
  auto handle = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(proc), nullptr, 0, &thread_id);
  if (!handle) return Error(get_system_error());

  return Ok(Thread { reinterpret_cast<Thread::Handle *>(handle), thread_id });
}

Result<Thread> spawn_thread (void *data, const Closure<u32 (void *)> &proc) {
  DWORD thread_id;
  auto handle = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(proc), data, 0, &thread_id);
  if (!handle) return Error(get_system_error());

  return Ok(Thread { reinterpret_cast<Thread::Handle *>(handle), thread_id });
}

Result<void> shutdown_thread (Thread &thread) {
  auto result = WaitForSingleObject(thread->handle, INFINITE);
  if (result == WAIT_FAILED) return Core::Error(get_system_error());

  if (!CloseHandle(thread->handle)) return Core::Error(get_system_error());

  thread->handle = nullptr;

  return Core::Ok();
}

u32 get_current_thread_id () {
  return GetCurrentThreadId();
}

}

