
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
  #define assert_caller(EXPR, CALLER)
#else
  #define assert(EXPR) ensure(EXPR)
  #define assert_msg(EXPR, MSG) ensure_msg(EXPR, MSG)

  #define assert_caller(EXPR, CALLER)                                 \
    do {                                                              \
      if (!static_cast<bool>(EXPR)) [[unlikely]] {                    \
        ::Fin::Core::assert_caller_hook(stringify(EXPR), nullptr, (CALLER)); \
      }                                                               \
    } while (0)
#endif

namespace Fin::Core {

static usize assert_get_length (const char *value) {
  usize length = 0;
  while (value[length]) length += 1;
  return length;
};

static usize assert_print_callsite (char *buffer, const Callsite_Info &callsite) {
  usize offset = 0;

  {
    const usize file_name_length = assert_get_length(callsite.file);
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
    auto length = assert_get_length(callsite.function);
    __builtin_memcpy(buffer + offset, callsite.function, length);
    offset += length;
  }

  return offset;
}

static usize assert_print_base_message (char *buffer, const char *expr, const Callsite_Info &assert_callsite) {
  usize offset = assert_print_callsite(buffer, assert_callsite);

  buffer[offset++] = ' ';
  buffer[offset++] = '-';
  buffer[offset++] = ' ';

  {
    const char msg[] = "ASSERT FAILED: ";
    __builtin_memcpy(buffer + offset, msg, Base::array_count_elements(msg) - 1);
    offset += Base::array_count_elements(msg) - 1;
  }
  
  {
    auto length = assert_get_length(expr);
    __builtin_memcpy(buffer + offset, expr, length);
    offset += length;
  }

  return offset;
}

[[noreturn]]
static void assert_trap (char *buffer, usize offset, const char *message) {
  if (!message) {
    buffer[offset++] = '\n';
    buffer[offset]   = '\0';

    trap(buffer);

    __builtin_unreachable();
  }

  buffer[offset++] = ',';
  buffer[offset++] = ' ';
  
  {
    auto length = assert_get_length(message);
    __builtin_memcpy(buffer + offset, message, length);
    offset += length;
  }

  buffer[offset++] = '\n';
  buffer[offset]   = '\0';

  trap(buffer, offset);
  __builtin_unreachable();
}

/*
  `assert_hook` allows us to inject custom logic into the core library, which is helpful in test environment,
  to capture corner cases.
 */
[[noreturn]]
static void assert_hook (const char *expr, const char *message, const Callsite_Info callsite = {}) {
  if (!expr) __builtin_trap();

  char buffer[1024];
  usize offset = assert_print_base_message(buffer, expr, callsite);

  assert_trap(buffer, offset, message);
}

[[noreturn]]
static void assert_caller_hook (const char *expr, const char *message, Callsite_Info caller, Callsite_Info callsite = {}) {
  if (!expr) __builtin_trap();

  char buffer[1024];
  usize offset = assert_print_base_message(buffer, expr, callsite);

  {
    const char msg[] = ", bad call at: ";
    __builtin_memcpy(buffer + offset, msg, Base::array_count_elements(msg) - 1);
    offset += Base::array_count_elements(msg) - 1;
  }

  offset += assert_print_callsite(buffer + offset, caller);

  assert_trap(buffer, offset, message);
}

}


