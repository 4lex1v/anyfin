#define FIN_THREADS_HPP_IMPL

#include "anyfin/threads.hpp"

#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

namespace Fin {

struct Thread {
  pthread_t handle;
};

template <typename T>
static Sys_Result<Thread> spawn_thread (const Invocable<void, T *> auto &proc, T *data) {
  pthread_t thread_id;
  auto thread_proc = [](void *arg) -> void* {
    auto *data_pair = reinterpret_cast<std::pair<decltype(proc), T*>*>(arg);
    data_pair->first(data_pair->second);
    delete data_pair;
    return nullptr;
  };
  
  auto *arg = new std::pair<decltype(proc), T*>(proc, data);
  int result = pthread_create(&thread_id, nullptr, thread_proc, arg);
  if (result != 0) {
    delete arg;
    return Error(get_system_error());
  }

  return Ok(Thread { thread_id });
}

static Sys_Result<Thread> spawn_thread (const Invocable<void> auto &proc) {
  pthread_t thread_id;
  auto thread_proc = [](void *arg) -> void* {
    auto *proc_ptr = reinterpret_cast<decltype(proc)*>(arg);
    (*proc_ptr)();
    delete proc_ptr;
    return nullptr;
  };
  
  auto *arg = new decltype(proc)(proc);
  int result = pthread_create(&thread_id, nullptr, thread_proc, arg);
  if (result != 0) {
    delete arg;
    return Error(get_system_error());
  }

  return Ok(Thread { thread_id });
}

static Sys_Result<void> shutdown_thread (Thread &thread) {
  int result = pthread_cancel(thread.handle);
  if (result != 0) {
    return Error(get_system_error());
  }
  return Ok();
}

static u32 get_current_thread_id () {
  return static_cast<u32>(pthread_self());
}

static void thread_sleep (usize milliseconds) {
  usleep(milliseconds * 1000);
}

}
