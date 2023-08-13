
#pragma once

#include "anyfin/base.hpp"
#include "anyfin/core/status_code.hpp"

template <typename T>
struct Result {
  Status_Code status;
  T           value;

  Result (Status_Code _status)
    : status { _status },
      value  {}
  {}

  Result (const T &_value)
    : status { Status_Code::Success },
      value  { _value }
  {}

  Result (T &&_value)
    : status { Status_Code::Success },
      value  { _value }
  {}

  fin_forceinline operator T& () { return value; }

  fin_forceinline T * operator & () { return &value; }
  fin_forceinline T * operator -> () { return &value; }

  fin_forceinline T operator * () const { return value; }

  operator Status_Code () const { return status; }
};
