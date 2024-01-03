
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/callsite.hpp"

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
static void assert_hook (const char *expr, const char *message, const Callsite_Info callsite = Callsite_Info());

}

#ifndef ASSERT_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "assert_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif


