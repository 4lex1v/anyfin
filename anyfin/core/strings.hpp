
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/allocator.hpp"
#include "anyfin/core/assert.hpp"
#include "anyfin/core/memory.hpp"
#include "anyfin/core/meta.hpp"
#include "anyfin/core/prelude.hpp"
#include "anyfin/core/trap.hpp"

namespace Fin::Core {

struct String;

template <typename T>
concept String_Convertible = requires (Allocator_View allocator, T a) {
  { to_string(a, allocator) } -> Same_Types<String>;
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

  constexpr String_View (): value { nullptr }, length { 0 } {}

  template <usize N>
  constexpr String_View (const char (&literal)[N])
    : value { literal }, length { N - 1 } {}

  constexpr String_View (const char *_value, usize _length)
    : value { _value }, length { _length } {}

  constexpr String_View (const String_View &view) = default;
  constexpr String_View (String_View &&view)      = default;

  constexpr explicit String_View (const char *_value)
    : value  { _value }, length { get_string_length(_value) } {}

  constexpr String_View (const String &string);

  constexpr String_View& operator = (const String_View &other) {
    this->value  = other.value;
    this->length = other.length;
    
    return *this;
  }

  constexpr operator         bool () const { return this->value != nullptr && this->length != 0; }
  constexpr operator const char * () const { return this->value; }
  
  char operator [] (usize idx) const { return value[idx]; }

  String_View operator + (int offset)   const { return String_View(this->value + offset, this->length - offset); }
  String_View operator + (u32 offset)   const { return String_View(this->value + offset, this->length - offset); }
  String_View operator + (usize offset) const { return String_View(this->value + offset, this->length - offset); }

  const char * begin () const { return value; }
  const char * end   () const { return value + length; }
};

static_assert(sizeof(String_View) == 16);

struct String {
  Allocator_View allocator;

  const char *value = nullptr;
  usize length      = 0;

  constexpr String () = default;

  constexpr String (Allocator auto &_allocator, const char *_value, const usize _length)
    : allocator { _allocator }, value { _value }, length { _length } {}

  constexpr String (String &&other)
    : allocator { move(other.allocator) },
      value     { other.value },
      length    { other.length }
  {
    other.value  = nullptr;
    other.length = 0;
  }

  String& operator = (const String &other) {
    this->~String();
    copy_string(other.allocator, *this, other);
    return *this;
  }

  String& operator = (String &&other) {
    this->~String();

    this->allocator = move(other.allocator);
    this->value     = other.value;
    this->length    = other.length;
    
    other.value  = nullptr;
    other.length = 0;

    return *this;
  }

  constexpr operator bool         () const { return this->length > 0; }
  constexpr operator const char * () const { return this->value; }

  constexpr char operator [] (usize idx) const { return value[idx]; }

  static String copy (Allocator auto &allocator, const String &string) {
    String new_string {};
    if (is_empty(string)) return new_string;
    copy_string(allocator, new_string, string);
    return new_string;
  }

  static String copy (Allocator auto &allocator, const String_View &view) {
    String new_string {};

    if (is_empty(view)) return new_string;

    copy_string(allocator, new_string, view);

    return new_string;
  }

  static String copy (Allocator auto &allocator, const char *value, const usize length) {
    return copy(allocator, String_View(value, length));
  }
  
  static String copy (Allocator auto &allocator, const char *value) {
    return copy(allocator, String_View(value));
  }

  const char * begin () const { return value; }
  const char * end   () const { return value + length; }

private:
  static void copy_string (Allocator auto &allocator, String &dest, const String_View &source) {
    auto buffer = reserve_memory(allocator, source.length + 1, alignof(char));

    memcpy(buffer, source.value, source.length);
    buffer[source.length] = '\0';

    dest.value  = reinterpret_cast<char *>(buffer);
    dest.length = source.length;
  }
};

static void destroy (String &string, const Callsite_Info info = Callsite_Info()) {
  free_reservation(string.allocator, (void*)string.value, info);
}

constexpr String_View::String_View (const String &string)
  : value { string.value }, length { string.length } {}

static bool is_empty (const String_View &view) {
  if (view.value == nullptr && view.length == 0) return true;
  return false;
}

constexpr bool ends_with (const String_View &view, const String_View &end) {
  if (end.length > view.length) return false;

  for (size_t i = 0; i < end.length; ++i) {
    if (view.value[view.length - end.length + i] != end.value[i]) {
      return false;
    }
  }

  return true; 
}

constexpr bool compare_strings (const String_View &left, const String_View &right) {
  // Check for equal length first
  if (left.length != right.length) return false;

  // Handle case where both are null or empty
  if (!left.value || !right.value) return left.value == right.value;

  // Compare string contents
  return compare_bytes<char>(left.value, right.value, left.length);
}

struct Format_String {
  /*
    Segment is a continious sequence of characters in the given format_string value.
    A new segment is create whenever the format_string has `%` placeholder, where the
    rendered value would be emplaced, or `%%`, tearing the format string in two parts.
   */
  struct Segment {
    enum struct Type: u32 { Text, Placeholder };

    Type type;

    // These fields are valid for Text segment types
    u16  start;
    u16  end;
  };

  constexpr static usize Segments_Count_Limit = 16;

  const char *format_string;
  usize       format_string_count;

  Segment segments[Segments_Count_Limit] {};

  usize   segments_count    = 0;
  usize   placeholder_count = 0;
  usize   reservation_size  = 0;

  template <usize STRING_LITERAL_LENGTH>
  consteval Format_String (const char (&format)[STRING_LITERAL_LENGTH])
    : format_string       { format },
      format_string_count { STRING_LITERAL_LENGTH - 1 }
  {
    static_assert(STRING_LITERAL_LENGTH > 1, "Empty string in formatter is not allowed");

    usize last = 0;
    for (usize idx = 0; idx < format_string_count; idx++) {
      if (format[idx] == '%') {
        u16 start  = last;
        u16 end    = idx;
        u16 length = end - start;

        auto text = Segment { .type = Segment::Type::Text, .start = start, .end = end };

        if (format[idx + 1] == '%') {
          text.end += 1;

          segments[segments_count]  = text;
          segments_count           += 1;
          reservation_size         += text.end - text.start;

          idx  += 1;
          last  = (idx + 1);

          continue;
        }

        // This is a valid scenario if a placeholder '%' is at the beginning of the string.
        if (length > 0) {
          segments[segments_count]  = text;
          segments_count           += 1;
          reservation_size         += text.end - text.start;
        }

        auto placeholder = Segment { .type = Segment::Type::Placeholder };
        
        segments[segments_count] = placeholder;
        segments_count    += 1;
        placeholder_count += 1;

        last = (idx + 1);
      }
    }

    if (last != format_string_count) {
      u16 start = static_cast<u16>(last);
      u16 end   = format_string_count;

      auto segment = Segment {.type = Segment::Type::Text, .start = start, .end = end };

      segments[segments_count]  = segment;
      segments_count           += 1;
      reservation_size         += end - start;
    }
  }
};

template <typename T>
concept Printable = String_Convertible<T> || requires (T value) {
  static_cast<String_View>(value);
};

template <Allocator Alloc_Type, Printable... Args>
static String format_string (Alloc_Type &allocator, const Format_String &format, Args&&... args) {
  constexpr usize N = sizeof...(Args);

  assert(format.placeholder_count == N);

  Scope_Allocator<Alloc_Type> local { allocator };

  auto render_value = [&local] (const auto &value) -> String_View {
    if constexpr (is_convertible<decltype(value), String_View>)
      return static_cast<String_View>(value);

    return to_string(value, local);
  };

  String_View arguments[N] { render_value(args)... };

  usize reservation_size = format.reservation_size + 1;
  for (auto &a: arguments) reservation_size += a.length;

  char buffer[reservation_size];

  usize cursor    = 0;
  usize arg_index = 0;
  for (usize idx = 0; idx < format.segments_count; idx++) {
    auto &segment = format.segments[idx];

    switch (segment.type) {
      case Format_String::Segment::Type::Text: {
        auto length = segment.end - segment.start;

        memcpy(buffer + cursor, format.format_string + segment.start, length);
        cursor += length;
        
        break;
      }
      case Format_String::Segment::Type::Placeholder: {
        assert(arg_index < N);
        auto entry = arguments + arg_index;

        memcpy(buffer + cursor, entry->value, entry->length);
        cursor += entry->length;

        arg_index += 1;
        
        break;
      }
    }
  }

  buffer[reservation_size - 1] = '\0';

  auto string = reserve_memory(allocator, reservation_size, alignof(char));
  memcpy(string, buffer, reservation_size);

  return String(allocator, reinterpret_cast<char *>(string), reservation_size - 1);
}

static auto to_string (const char value, Allocator auto &allocator) {
  auto memory = reserve_memory(allocator, sizeof(char) + 1, alignof(char));
  if (!memory) trap("Out of memory");

  memory[0] = value;
  memory[1] = '\0';

  return String(allocator, reinterpret_cast<char *>(memory), 1);
}

template <usize N>
static auto to_string (const char (&value)[N], Allocator auto &allocator) {
  auto buffer = reinterpret_cast<char *>(reserve_memory(allocator, N, alignof(char)));
  if (!buffer) trap("Out of memory");

  buffer[N] = '\0';

  return String(allocator, buffer, N - 1);
}

template <Integral I>
static auto to_string (I value, Allocator auto &allocator) {
  char buffer[20];
  usize offset;
  
  bool is_negative = false;
  if constexpr (is_signed<I>) {
    is_negative = value < 0;
    value       = -value; 
  }

  do {
    const auto digit = value % 10;
    buffer[offset++] = '0' + digit;
    value /= 10;
  } while (value != 0);

  if constexpr (is_signed<I>) {
    if (is_negative) buffer[offset++] = '-';
  }

  auto string = reinterpret_cast<char *>(reserve_memory(allocator, offset + 1));
  for (usize idx = offset; idx > 0; idx--) {
    string[offset - idx] = buffer[idx];
  }
  string[offset + 1] = '\0';
  
  return String(allocator, string, offset);
}

namespace iterator {

struct Split_Iterator {
  String_View string;
  char        separator;

  constexpr Split_Iterator (const String_View& view, char sep)
    : string(view), separator(sep) {}

  struct Iterator {
    const char* str_;
    const usize length_;
    const char* start_ = nullptr;
    const char* current_ = nullptr;
    char delim_;
    bool end_;

    constexpr Iterator (const char* str, usize len, char delim, bool end = false)
      : str_(str), length_(len), delim_(delim), end_(end)
    {
      if (!end) find_next();
    }

    String_View operator * () const {
      return {start_, static_cast<usize>(current_ - start_)};
    }

    Iterator& operator++() {
      find_next();
      return *this;
    }

    bool operator != (const Iterator& other) const {
      return end_ != other.end_;
    }

    void find_next () {
      if (current_ == nullptr) {
        current_ = str_;
        start_ = str_;
      }
      else if (current_ - str_ < length_) {
        current_++;
        start_ = current_;
      }
      else {
        end_ = true;
        return;
      }

      while (current_ - str_ < length_ && *current_ != delim_) {
        current_++;
      }

      if (current_ - str_ == length_) {
        end_ = true;
      }
    }
  };

  Iterator begin () const { return Iterator(string.value, string.length, separator); }
  Iterator end   () const { return Iterator(string.value, string.length, separator, true); }
};

constexpr Split_Iterator split (const String_View &string, const char separator) {
  return Split_Iterator(string, separator);
}

};


}
  
