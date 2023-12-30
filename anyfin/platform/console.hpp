
#pragma once

#include "anyfin/core/arena.hpp"
#include "anyfin/core/strings.hpp"
#include "anyfin/core/meta.hpp"

namespace Fin::Core {

void console_print_message (const String_View &string);

template <usize MEMORY_SIZE = 1024, typename... Args>
static void print (Format_String &&format, Args&&... args) {
  u8 stack_memory[MEMORY_SIZE];
  Memory_Arena local { stack_memory, MEMORY_SIZE };

  auto message = format_string(local, forward<Format_String>(format), forward<Args>(args)...);

  console_print_message(message);
}

}
