
#pragma once

#include "anyfin/base.hpp"

struct Status_Code {
  enum struct Value: u32 {
    Success,
    System_Error,
  };

  using enum Value;

  Value       value      = Value::Success;
  u32         error_code = 0;
  const char *details    = nullptr;

  Status_Code () = default;

  Status_Code (Value _value)
    : value { _value }
  {}

  Status_Code (Value _value, u32 _code)
    : value      { _value },
      error_code { _code }
  {}

  Status_Code (Value _value, const char *_details)
    : value { _value },
      details { _details }
  {}

  Status_Code (Value _value, u32 _code, const char *_details)
    : value      { _value },
      error_code { _code },
      details    { _details }
  {}
};
