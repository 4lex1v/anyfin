
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/allocator.hpp"
#include "anyfin/core/strings.hpp"
#include "anyfin/core/list.hpp"
#include "anyfin/core/slice.hpp"

namespace Fin::Core {

struct String_Builder {
  List<String_View> sections;

  usize length = 0;

  String_Builder (Allocator auto &_allocator)
    : sections { _allocator } {}

  void add (const String_View &value) {
    if (value.length == 0) return;

    list_push_copy(this->sections, value);

    this->length += value.length;
  }

  void add (const Slice<String_View> &parts) { for (auto value: parts)  add(value); }
  void add (const Slice<String> &parts)      { for (auto &value: parts) add(value); }

  void operator += (const String_View &value)         { add(value); }
  void operator += (const Slice<String_View> &values) { add(values); }

  void operator += (const String &string)        { add(string); }
  void operator += (const Slice<String> &values) { add(values); }
};

static String build_string (Can_Reserve_Memory auto &allocator, const String_Builder &builder, bool use_separator, char separator) {
  if (!builder.length) return {};
    
  auto reservation_size = builder.length + 1;
  if (use_separator) reservation_size += builder.sections.count;

  auto buffer = reinterpret_cast<char *>(reserve_memory(allocator, reservation_size, alignof(char)));

  usize offset = 0;
  for (auto section: builder.sections) {
    assert(section.length > 0);
      
    memcpy(buffer + offset, section.value, section.length);
    offset += section.length;

    if (use_separator) buffer[offset++] = separator;
  }

  buffer[offset] = '\0';  

  return String(allocator, buffer, offset);
}

static String build_string (Can_Reserve_Memory auto &allocator, const String_Builder &builder) {
  return build_string(allocator, builder, false, 0);
}

static String build_string_with_separator (Can_Reserve_Memory auto &allocator, const String_Builder &builder, char separator) {
  return build_string(allocator, builder, true, separator);
}

}
