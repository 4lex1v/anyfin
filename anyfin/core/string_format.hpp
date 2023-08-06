
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/strings.hpp"

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

template <String_Convertible... Args>
static String format_string (Memory_Arena *arena, Format_String format, Args&&... args) {
  constexpr usize N = sizeof...(Args);

  /*
    I wish this could be done at compile time :sadface:
   */
  assert(format.placeholder_count == N);

  String arguments[N] { make_string(arena, args)... };

  usize reservation_size = format.reservation_size + 1;
  for (auto &a: arguments) reservation_size += a.length;

  auto buffer = reserve_array<char>(arena, reservation_size);
  if (!buffer) todo();

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

  return String { buffer, reservation_size - 1 };
}

