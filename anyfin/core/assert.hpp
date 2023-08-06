
#pragma once

#include "anyfin/base.hpp"

/*
  `assert_hook` allows us to inject custom logic into the core library, which is helpful in test environment,
  to capture corner cases.
 */
void assert_hook (const char *filename, usize line, const char *function, const char *expr, const char *message);

/*
  `ensure_msg` is an assert-kind macro that, unlike `assert`, ALWAYS triggers regardless of the build type.
 */
#define ensure_msg(EXPR, MSG)                                              \
  do {                                                                     \
    if (!static_cast<bool>(EXPR)) {                                        \
      assert_hook(__FILE__, __LINE__, __FUNCTION__, stringify(EXPR), MSG); \
    }                                                                      \
  } while (0)

#define ensure(EXPR) ensure_msg(EXPR, nullptr)

#ifdef DISABLE_ASSERTIONS
  #define assert(EXPR)
  #define assert_msg(EXPR, MSG)
#else
  #define assert(EXPR) ensure(EXPR)
  #define assert_msg(EXPR, MSG) ensure_msg(EXPR, MSG)
#endif
