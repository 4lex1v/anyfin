
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/allocator.hpp"
#include "anyfin/core/arena.hpp"
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

constexpr const char * string_adapt (Byte_Type auto *values) {
  using T = raw_type<decltype(values)>;
  auto consted = const_cast<const T *>(values);

  struct Cast_Workaround {
    union {
      const char *as_char;
      const u8   *as_uchar;
    };

    constexpr Cast_Workaround (const u8 *_uchar): as_uchar { _uchar } {}
    constexpr Cast_Workaround (const char *_char): as_char { _char } {}
  };

  Cast_Workaround cast { consted };
  return cast.as_char;
}

struct String_View {
  const char *value;
  usize       length;

  constexpr String_View (): value { nullptr }, length { 0 } {}

  constexpr String_View (const String_View &other)
    : value { other.value }, length { other.length } {}

  template <usize N>
  constexpr String_View (Byte_Array<N> auto (&literal)[N])
    : value { string_adapt(literal) }, length { N - 1 } {}

  constexpr String_View (Byte_Pointer auto _value, usize _length)
    : value { string_adapt(_value) }, length { _length } {}

  constexpr String_View (Byte_Pointer auto _value) {
    this->value  = string_adapt(_value);
    this->length = get_string_length(this->value);
  }

  constexpr String_View (const String &string);

  constexpr String_View& operator = (String_View other) {
    this->value  = other.value;
    this->length = other.length;
    
    return *this;
  }

  constexpr operator bool         (this auto self) { return self.value && self.length; }
  constexpr operator const char * (this auto self) { return self.value; }
  
  constexpr auto operator [] (this auto self, usize idx) { return self.value[idx]; }

  constexpr auto operator + (this auto self, Integral auto offset) {
    assert(offset <= self.length);
    return String_View(self.value + offset, self.length - offset);
  }

  constexpr const char * begin (this auto self) { return self.value; }
  constexpr const char * end   (this auto self) { return self.value + self.length; }
};

static_assert(sizeof(String_View) == 16);

struct String {
  Allocator_View allocator;

  const char *value = nullptr;
  usize length      = 0;

  constexpr String () = default;

  template <usize N>
  constexpr String (Allocator auto &_allocator, Byte_Array<N> auto (&array)[N])
    : allocator { _allocator }, value { string_adapt(array) }, length { N - 1 } {}

  constexpr String (Allocator auto &_allocator, Byte_Pointer auto _value, const usize _length)
    : allocator { _allocator }, value { string_adapt(_value) }, length { _length } {}

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

  constexpr auto operator [] (this auto &&self, usize idx) { return self.value[idx]; }

  static String copy (Allocator auto &allocator, String_View view, Callsite_Info callsite = {}) {
    if (is_empty(view)) return {};

    String new_string;
    copy_string(allocator, new_string, view);

    return new_string;
  }

  static String copy (Allocator auto &allocator, const String &string, Callsite_Info callsite = {}) {
    return copy(allocator, string, callsite);
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
    auto buffer = reserve_memory(allocator, source.length + 1, alignof(char), callsite);

    copy_memory(buffer, source.value, source.length);
    buffer[source.length] = '\0';

    dest.value  = reinterpret_cast<char *>(buffer);
    dest.length = source.length;
  }
};

constexpr String_View::String_View (const String &string)
  : value { string.value }, length { string.length } {}

static void destroy (String &string, const Callsite_Info info = Callsite_Info()) {
  free_reservation(string.allocator, (void*)string.value, info);
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
    else return to_string(value, local);
  };

  String_View arguments[N] { render_value(args)... };

  usize reservation_size = format.reservation_size + 1;
  for (auto &a: arguments) reservation_size += a.length;

  /*
    TODO: I'm planning to rewrite this whole formatter thing to avoid dynamic arrays, for now going to stub
    whatever is sufficient here to do the job.
   */
  // char buffer[reservation_size];
  Local_Arena<2048> arena;
  char *buffer = reserve_memory(arena, 2048);

  usize cursor    = 0;
  usize arg_index = 0;
  for (usize idx = 0; idx < format.segments_count; idx++) {
    auto &segment = format.segments[idx];

    switch (segment.type) {
      case Format_String::Segment::Type::Text: {
        auto length = segment.end - segment.start;

        copy_memory(buffer + cursor, format.format_string + segment.start, length);
        cursor += length;
        
        break;
      }
      case Format_String::Segment::Type::Placeholder: {
        assert(arg_index < N);
        auto entry = arguments + arg_index;

        copy_memory(buffer + cursor, entry->value, entry->length);
        cursor += entry->length;

        arg_index += 1;
        
        break;
      }
    }
  }

  buffer[reservation_size - 1] = '\0';

  auto string = reserve_memory(allocator, reservation_size, alignof(char));
  if (is_empty(string)) trap("Out of memory");

  copy_memory(string, buffer, reservation_size);

  return String(allocator, reinterpret_cast<char *>(string), reservation_size - 1);
}

static auto to_string (const char value, Allocator auto &allocator) {
  auto memory = reserve_memory(allocator, sizeof(char) + 1, alignof(char));
  if (!memory) trap("Out of memory");

  memory[0] = value;
  memory[1] = '\0';

  return String(allocator, reinterpret_cast<char *>(memory), 1);
}

static auto to_string (Byte_Pointer auto value, Allocator auto &allocator, Callsite_Info callsite = {}) {
  return String::copy(allocator, value, callsite);
}

template <usize N>
static auto to_string (Byte_Array<N> auto (&value)[N], Allocator auto &allocator) {
  return String::copy(allocator, value, N - 1);
}

template <Integral I>
static auto to_string (I value, Allocator auto &allocator) {
  char buffer[20];
  usize offset;
  
  bool is_negative = false;
  if constexpr (Signed_Integral<I>) {
    is_negative = value < 0;
    value       = -value; 
  }

  do {
    const auto digit = value % 10;
    buffer[offset++] = '0' + digit;
    value /= 10;
  } while (value != 0);

  if constexpr (Signed_Integral<I>) {
    if (is_negative) buffer[offset++] = '-';
  }

  auto string = reserve_memory<char>(allocator, offset + 1);
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
  
