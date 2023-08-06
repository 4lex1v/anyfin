
#pragma once

#include "anyfin/base.hpp"

using Assert_Hook = void(*)(const char *filename, usize line, const char *function, const char *message);

/*
  `assert_hook` allows us to inject custom logic into the core library, which is helpful in test environment,
  to capture corner cases.
 */
Assert_Hook assert_hook;

/*
  Ensure is an assert-kind macro that, unlike `assert`, ALWAYS triggers regardless of the build type.
 */
#define ensure(EXPR) \
  do {               \
    if (!static_cast<bool>(EXPR)) { \
      assert_hook(__FILE__, __LINE__, __FUNCTION__, stringify(EXPR))                 \
    }                       \
  } while(0)

#ifdef DISABLE_ASSERTIONS
  #define assert(EXPR)
#else
  #define assert(EXPR) ensure(EXPR)
#endif