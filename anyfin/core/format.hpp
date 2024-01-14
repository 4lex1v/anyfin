
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/strings.hpp"
#include "anyfin/core/list.hpp"

namespace Fin::Core {

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

template <Allocator Alloc_Type, Printable... Args>
static String format_string (Alloc_Type &allocator, const Format_String &format, Args&&... args) {
  constexpr usize N = sizeof...(Args);

  assert(N == format.placeholder_count);

  Core::List<String> free_list { allocator };
  defer { alloc_destroy<Alloc_Type>(free_list); };

  auto render_value = [&free_list, &allocator] (const auto &value) -> String_View {
    if constexpr (is_convertible<decltype(value), String_View>)
      return static_cast<String_View>(value);
    else {
      String rendered = to_string(value, allocator);
      String_View ref = rendered;
      list_push(free_list, move(rendered));
      return ref;
    }
  };

  const String_View arguments[N] { render_value(args)... }; 

  usize reservation_size = format.reservation_size + 1;
  for (auto &a: arguments) reservation_size += a.length;

  auto buffer = reserve(allocator, reservation_size);
  if (!buffer) trap("Allocator is out of memory");

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
        auto entry = arguments + arg_index;

        copy_memory(buffer + cursor, entry->value, entry->length);
        cursor += entry->length;

        arg_index += 1;
        
        break;
      }
    }
  }

  buffer[reservation_size - 1] = '\0';

  return String(allocator, buffer, reservation_size - 1);
}

static auto to_string (bool value, Allocator auto &allocator) {
  return String::copy(allocator, value ? String_View("true") : String_View("false"));
}

static auto to_string (char value, Allocator auto &allocator) {
  auto memory = reserve(allocator, sizeof(char) + 1, alignof(char));
  if (!memory) trap("Out of memory");

  memory[0] = value;
  memory[1] = '\0';

  return String(allocator, memory, 1);
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
  usize offset = 0;
  
  bool is_negative = false;
  if constexpr (Signed_Integral<I>) {
    if (value < 0) {
      is_negative = true;
      value       = -value; 
    }
  }

  do {
    const auto digit = value % 10;
    buffer[offset++] = '0' + digit;
    value /= 10;
  } while (value != 0);

  if constexpr (Signed_Integral<I>) {
    if (is_negative) buffer[offset++] = '-';
  }

  auto string = reserve<char>(allocator, offset + 1);
  for (usize idx = 0; idx < offset; idx++) {
    string[idx] = buffer[offset - 1 - idx];
  }
  string[offset] = '\0';
  
  return String(allocator, string, offset);
}

}
