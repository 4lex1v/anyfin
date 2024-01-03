
#pragma once

#include "anyfin/core/meta.hpp"

#include "anyfin/platform/platform.hpp"

namespace Fin::Platform {

struct Thread {
  struct Handle;

  Handle *handle;
  u32     id;
};

static Result<Thread> spawn_thread (const Core::Invocable<void> auto &proc);

template <typename T>
static Result<Thread> spawn_thread (const Core::Invocable<void, T *> auto &proc, T *data);

static Result<void> shutdown_thread (Thread &thread);

static u32 get_current_thread_id ();

}

#ifndef THREADS_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "threads_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
