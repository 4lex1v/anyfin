
#pragma once

#include "anyfin/platform/platform.hpp"

namespace Fin::Platform {

using Thread_Proc = u32 (*) ();

struct Thread {
  struct Handle;

  Handle *handle;
  u32     id;
};

static Result<Thread> spawn_thread (Thread_Proc proc);

static Result<Thread> spawn_thread (void *data, Thread_Proc proc);

static Result<void> shutdown_thread (Thread &thread);

static u32 get_current_thread_id ();

}

#ifndef THREADS_API_IMPL
  #ifdef PLATFORM_WIN32
    #include "threads_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
