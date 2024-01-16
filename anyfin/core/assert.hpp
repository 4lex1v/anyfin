
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/callsite.hpp"
#include "anyfin/core/trap.hpp"

/*
  `ensure_msg` is an assert-kind macro that, unlike `assert`, ALWAYS triggers regardless of the build type.
 */
#define ensure_msg(EXPR, MSG)                         \
  do {                                                \
    if (!static_cast<bool>(EXPR)) [[unlikely]] {      \
      ::Fin::Core::assert_hook(stringify(EXPR), MSG); \
    }                                                 \
  } while (0)

#define ensure(EXPR) ensure_msg(EXPR, nullptr)

#ifdef DISABLE_ASSERTIONS
  #define assert(EXPR)
  #define assert_msg(EXPR, MSG)
#else
  #define assert(EXPR) ensure(EXPR)
  #define assert_msg(EXPR, MSG) ensure_msg(EXPR, MSG)
#endif

namespace Fin::Core {

/*
  `assert_hook` allows us to inject custom logic into the core library, which is helpful in test environment,
  to capture corner cases.
 */
[[noreturn]]
static void assert_hook (const char *expr, const char *message, const Callsite_Info callsite = {}) {
  if (!expr) __builtin_trap();

  const auto get_length = [] (const char *value) -> usize {
    usize length = 0;
    while (value[length]) length += 1;
    return length;
  };

  char buffer[1024];
  usize offset = 0;

  {
    const usize file_name_length = get_length(callsite.file);
    __builtin_memcpy(buffer, callsite.file, file_name_length);
    offset += file_name_length;
  }

  buffer[offset++] = '(';

  {
    char temp[20];
    usize temp_offset = 0;

    usize value = callsite.line;

    do {
      const auto digit = value % 10;
      temp[temp_offset++] = '0' + digit;
      value /= 10;
    } while (value != 0);

    for (usize idx = 0; idx < temp_offset; idx++) {
      buffer[offset++] = temp[temp_offset - 1 - idx];
    }
  }

  buffer[offset++] = ')';
  buffer[offset++] = ':';

  {
    auto length = get_length(callsite.function);
    __builtin_memcpy(buffer + offset, callsite.function, length);
    offset += length;
  }

  buffer[offset++] = ' ';
  buffer[offset++] = '-';
  buffer[offset++] = ' ';
  
  {
    auto length = get_length(expr);
    __builtin_memcpy(buffer + offset, expr, length);
    offset += length;
  }

  if (!message) {
    buffer[offset++] = '\n';
    buffer[offset] = '\0';
    trap(buffer);
    __builtin_unreachable();
  }

  buffer[offset++] = ',';
  buffer[offset++] = ' ';
  
  {
    auto length = get_length(message);
    __builtin_memcpy(buffer + offset, message, length);
    offset += length;
  }

  buffer[offset++] = '\n';

  trap(buffer);
  __builtin_unreachable();
}

}


