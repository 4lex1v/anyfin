
#pragma once

#include "assert.hpp"

/*
  It's intentional that the string's length value doesn't include the terminating zero.
  Nevertheless other parts of the system that create of copy strings around still put a terminating zero
  to be seamlessly compatible with C API.
 */
struct String {
  const char *value  = nullptr;
  usize       length = 0;

  constexpr String () = default;
  constexpr String (const char *_value, usize _length)
    : value  { _value },
      length { _length }
  {}

  constexpr String (const char *_value)
    : value { _value }
  {
    if (this->value == nullptr) return;
    if (this->value[0] == '\0') return;
    
    // We know that the first byte is not \0
    while (this->value[++this->length]);
  }

  char operator [] (usize idx) const {
    return value[idx];
  }
};

constexpr bool is_empty_string (const String &string) {
  return string.length == 0;
}

constexpr bool compare_strings (const String &first, const String &second) {
  if (first.length != second.length) return false;
  
  for (usize idx = 0; idx < first.length; idx++) {
    if (first[idx] != second[idx]) return false;
  }

  return true;
}

