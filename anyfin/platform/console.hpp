
#pragma once

#include "anyfin/core/arena.hpp"
#include "anyfin/core/format.hpp"
#include "anyfin/core/meta.hpp"

#include "anyfin/platform/platform.hpp"

namespace Fin::Core {

static Fin::Platform::Result<void> print (String_View string);

template <typename... Args>
static Fin::Platform::Result<void> print (Allocator auto &allocator, Format_String &&format, Args&&... args) {
  auto message = format_string(allocator, move(format), forward<Args>(args)...);
  return print(message);
}

template <usize MEMORY_SIZE = 2048, typename... Args>
static Fin::Platform::Result<void> print (Format_String &&format, Args&&... args) {
  Local_Arena<MEMORY_SIZE> local;
  return print(local.arena, move(format), forward<Args>(args)...);
}

template <usize MEMORY_SIZE = 2048, typename... Args>
static Fin::Platform::Result<void> log_internal (Format_String &&format, Callsite_Info callsite, Args&&... args) {
  Local_Arena<MEMORY_SIZE> local;
  return print(local.arena, move(format), callsite, forward<Args>(args)...);
}

#define log(FMT, ...) log_internal("%: " FMT, {}, ##__VA_ARGS__)

}

#ifndef CONSOLE_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "console_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
