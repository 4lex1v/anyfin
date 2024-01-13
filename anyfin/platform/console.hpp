
#pragma once

#include "anyfin/core/arena.hpp"
#include "anyfin/core/format.hpp"
#include "anyfin/core/meta.hpp"

#include "anyfin/platform/platform.hpp"

namespace Fin::Core {

static Fin::Platform::Result<void> print (String_View string);

template <usize MEMORY_SIZE = 1024, typename... Args>
static Fin::Platform::Result<void> print (Format_String &&format, Args&&... args) {
  Local_Arena<MEMORY_SIZE> local;
  auto message = format_string(local.arena, move(format), forward<Args>(args)...);
  return print(message);
}

}

#ifndef CONSOLE_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "console_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
