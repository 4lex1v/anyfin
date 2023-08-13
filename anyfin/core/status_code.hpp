
#pragma once

#include "anyfin/base.hpp"

struct Status_Code {
  enum struct Value: u32 {
    Success,
    System_Error,
    Runtime_Error,
    Fatal_Error,
  };

  using enum Value;

  Value value         = Success;
  u32   error_class   = 0;
  s32   error_code    = 0;
  const char *details = nullptr;

  Status_Code () = default;

  Status_Code (Value _value)
    : value { _value }
  {}

  Status_Code (Value _value, u32 _class)
    : value       { _value },
      error_class { _class }
  {}

  Status_Code (Value _value, const char *_details)
    : value   { _value },
      details { _details }
  {}

  Status_Code (Value _value, u32 _class, const char *_details)
    : value       { _value },
      error_class { _class },
      details     { _details }
  {}

  bool operator ! () const {
    return value != Success;
  }
};

#define check_status(EXPR)                              \
  do {                                                  \
    const auto status = static_cast<Status_Code>(EXPR); \
    if (!status) return status;                         \
  } while (0)

