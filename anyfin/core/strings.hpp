
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/allocator.hpp"
#include "anyfin/core/memory.hpp"
#include "anyfin/core/meta.hpp"
#include "anyfin/core/prelude.hpp"

namespace Fin::Core {

struct String_View;
struct String;

template <typename T>
concept String_Convertible = requires (Allocator_View allocator, T a) {
  { to_string(a, allocator) } -> Same_Types<String>;
};

template <typename T>
concept Printable = String_Convertible<T> || requires (T value) {
  static_cast<String_View>(value);
};

// NOTE: Won't include the terminating null into the length
constexpr usize get_string_length (const char *value) {
  if ((value == nullptr) || (value[0] == '\0')) return 0;

  usize length = 0;
  while (value[length]) length += 1;

  return length;
}

struct String_View {
  const char *value;
  usize       length;

  fin_forceinline
  constexpr String_View (): value { nullptr }, length { 0 } {}

  template <usize N>
  fin_forceinline
  constexpr String_View (Byte_Array<N> auto (&literal)[N])
    : value { cast_bytes(literal) }, length { N - 1 } {}

  fin_forceinline
  constexpr String_View (Byte_Pointer auto _value, usize _length)
    : value { cast_bytes(_value) }, length { _length } {}

  fin_forceinline
  constexpr String_View (Byte_Pointer auto _value) {
    this->value  = cast_bytes(_value);
    this->length = get_string_length(this->value);
  }

  constexpr String_View (const String &string);

  fin_forceinline
  constexpr String_View& operator = (String_View other) {
    this->value  = other.value;
    this->length = other.length;
    
    return *this;
  }

  constexpr operator bool         (this auto self) { return self.value && self.length; }
  constexpr operator const char * (this auto self) { return self.value; }
  
  constexpr auto operator [] (this auto self, usize idx) { return self.value[idx]; }

  constexpr auto operator + (this auto self, Integral auto offset) {
    return String_View(self.value + offset, self.length - offset);
  }

  constexpr const char * begin (this auto self) { return self.value; }
  constexpr const char * end   (this auto self) { return self.value + self.length; }
};

static_assert(sizeof(String_View) == 16);

static void destroy (String &string, const Callsite_Info info = {});

struct String {
  Allocator_View allocator;

  const char *value = nullptr;
  usize length      = 0;

  constexpr String () = default;

  template <usize N>
  constexpr String (Allocator auto &_allocator, Byte_Array<N> auto (&array)[N])
    : allocator { _allocator }, value { cast_bytes(array) }, length { N - 1 } {}

  constexpr String (Allocator auto &_allocator, Byte_Pointer auto _value, const usize _length)
    : allocator { _allocator }, value { cast_bytes(_value) }, length { _length } {}

  constexpr String (const String &) = delete;

  constexpr String (String &&other)
    : allocator { other.allocator },
      value     { other.value },
      length    { other.length }
  {
    other.allocator = {};
    other.value     = nullptr;
    other.length    = 0;
  }

  String& operator = (const String &other) {
    destroy(*this);
    copy_string(other.allocator, *this, other);
    return *this;
  }

  String& operator = (String &&other) {
    destroy(*this);

    this->allocator = move(other.allocator);
    this->value     = other.value;
    this->length    = other.length;
    
    other.allocator = {};
    other.value     = nullptr;
    other.length    = 0;

    return *this;
  }

  constexpr operator bool         () const { return this->length > 0; }
  constexpr operator const char * () const { return this->value; }

  constexpr auto operator [] (this auto &&self, usize idx) { return self.value[idx]; }

  static String copy (Allocator auto &allocator, String_View view, Callsite_Info callsite = {}) {
    if (is_empty(view)) return {};

    String new_string;
    new_string.allocator = allocator;
    copy_string(allocator, new_string, view);

    return new_string;
  }

  static String copy (Allocator auto &allocator, const String &string, Callsite_Info callsite = {}) {
    return copy(allocator, static_cast<String_View>(string), callsite);
  }
  
  static String copy (Allocator auto &allocator, const char *value, const usize length, Callsite_Info callsite = {}) {
    return copy(allocator, String_View(value, length), callsite);
  }
  
  static String copy (Allocator auto &allocator, const char *value, Callsite_Info callsite = {}) {
    return copy(allocator, String_View(value), callsite);
  }

  const char * begin () const { return value; }
  const char * end   () const { return value + length; }

private:
  static void copy_string (Allocator auto &allocator, String &dest, String_View source, Callsite_Info callsite = {}) {
    auto buffer = reserve(allocator, source.length + 1, alignof(char), callsite);

    copy_memory(buffer, source.value, source.length);
    buffer[source.length] = '\0';

    dest.value  = reinterpret_cast<char *>(buffer);
    dest.length = source.length;
  }
};

fin_forceinline
constexpr String_View::String_View (const String &string)
  : value { string.value }, length { string.length } {}

static void destroy (String &string, const Callsite_Info info) {
  if (string.value) free(string.allocator, (void*)string.value, false, info);
}

constexpr String copy (const String &string, const Callsite_Info callsite = {}) {
  return String::copy(string.allocator, string, callsite);
}

constexpr bool is_empty (String_View view) {
  if (view.value == nullptr && view.length == 0) return true;
  return false;
}

constexpr bool starts_with (String_View view, String_View start) {
  if (start.length > view.length) return false;

  for (size_t i = 0; i < start.length; ++i) {
    if (view.value[i] != start.value[i]) return false;
  }

  return true;
}

constexpr bool ends_with (String_View view, String_View end) {
  if (end.length > view.length) return false;

  for (size_t i = 0; i < end.length; ++i) {
    if (view.value[view.length - end.length + i] != end.value[i]) {
      return false;
    }
  }

  return true; 
}

constexpr bool has_substring (String_View text, String_View value) {
  if (value.length == 0)          return true;
  if (text.length < value.length) return false;

  for (size_t i = 0; i <= text.length - value.length; i++) {
    if (compare_bytes(text.value + i, value.value, value.length)) return true;
  }

  return false;
}

constexpr bool compare_strings (String_View left, String_View right) {
  if (left.length != right.length) return false;
  if (!left.value || !right.value) return left.value == right.value;

  return compare_bytes<char>(left.value, right.value, left.length);
}

struct split_string {
  const char *cursor = nullptr;
  const char *end    = nullptr;

  char separator;
  
  constexpr split_string (String_View _string, char _separator)
    : separator { _separator }
  {
    if (is_empty(_string)) return;
      
    cursor = _string.value;
    end    = cursor + _string.length;
  }

  constexpr void for_each (const Invocable<void, String_View> auto &func) {
    while (end_reached() == false) {
      if (skip_consequtive_separators()) return;

      auto next_position = get_character_offset(cursor, end - cursor, separator);
      assert(next_position != cursor); // We've just skipped all separators, this wouldn't make any sense.

      if (next_position) {
        func(String_View(cursor, next_position - cursor));

        cursor = next_position;
        continue;
      }
      
      func(String_View(cursor, end - cursor));
      
      cursor = end;
    }
  }

  constexpr bool skip_consequtive_separators () {
    while (!end_reached()) {
      if (*cursor != separator) return false;
      cursor += 1;
    }

    return true;
  }

  constexpr bool end_reached () { return cursor == end; }
};

}
  
