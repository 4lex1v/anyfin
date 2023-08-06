
#pragma once

#include "anyfin/base.hpp"
#include "anyfin/strings.hpp"

template <String_Convertible... Args>
static void print (Format_String &&format, Args&&... args) {
  auto local = *arena;
  auto message = format_string(&local, forward<Format_String>(format), forward<Args>(args)...);

  void platform_print_message (const String &message);
  platform_print_message(message);
}

