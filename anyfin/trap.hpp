
#pragma once

#include "anyfin/base.hpp"
#include "anyfin/callsite.hpp"

namespace Fin {

[[noreturn]] void trap (const char *message, usize message_size, Callsite callsite = {});

[[noreturn]] static void trap (const char *message, Callsite callsite = {}) {
  usize length = 0;
  while (message[length]) length += 1;
  trap(message, length, callsite);
}

template <usize N>
[[noreturn]] static void trap (const char (&msg)[N], Callsite callsite = {}) {
  trap(msg, N - 1, callsite);
}

}
