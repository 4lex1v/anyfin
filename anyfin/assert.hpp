
#pragma once

#include "base.hpp"

#ifdef DISABLE_ASSERTIONS

#define assert(EXPR)

#else 

#define assert(EXPR)                                              \
  do {                                                            \
    void platform_raise_error_and_halt (const char *);            \
    if (!static_cast<bool>(EXPR))                                 \
      raise_error_and_halt("Assertion failed: " stringify(EXPR)); \
  } while (0)

#endif
