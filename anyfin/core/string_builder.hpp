
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/allocator.hpp"
#include "anyfin/core/strings.hpp"
#include "anyfin/core/list.hpp"

namespace Fin::Core {

struct String_Builder {
  List<String_View> sections;
  usize length = 0;

  constexpr String_Builder (Allocator auto &_allocator)
    : sections { _allocator } {}

  constexpr auto& add (this auto &self, String_View value) {
    if (value.length == 0) return self;

    list_push(self.sections, move(value));
    self.length += value.length;

    return self;
  }

  constexpr auto& operator += (this String_Builder &self, const String_View &value) { self.add(value); return self; }
  constexpr auto& operator += (this String_Builder &self, const String      &value) { self.add(value); return self; }
  constexpr auto& operator += (this String_Builder &self, const char        *value) { self.add(value); return self; }

  constexpr auto& operator += (this String_Builder &self, const Iterable<String_View> auto &values) { for (auto &value: values) self.add(value); return self; }
  constexpr auto& operator += (this String_Builder &self, const Iterable<String>      auto &values) { for (auto &value: values) self.add(value); return self; }

  constexpr auto& add (this String_Builder &self, Allocator auto &allocator, Printable auto &&... args) {
    const auto print = [&] (auto &&arg) {
      if constexpr (Convertible_To<decltype(arg), String_View>)
        return static_cast<String_View>(arg);
      else return to_string(arg, allocator);
    };

    (self.add(print(args)), ...);

    return self;
  }
};

static void destroy (String_Builder &builder, Callsite_Info callsite = {}) {
  destroy(builder.sections, callsite);
}

static String build_string (Allocator auto &allocator, const String_Builder &builder, bool use_separator, char separator) {
  if (!builder.length) return {};
    
  auto reservation_size = builder.length + 1;
  if (use_separator) reservation_size += builder.sections.count;

  auto buffer = reserve(allocator, reservation_size);

  usize offset = 0;
  builder.sections.for_each([&] (auto section) {
    copy_memory(buffer + offset, section.value, section.length);
    offset += section.length;

    if (use_separator) buffer[offset++] = separator;
  });

  buffer[offset] = '\0';  

  return String(allocator, buffer, offset);
}

static String build_string (Allocator auto &allocator, const String_Builder &builder) {
  return build_string(allocator, builder, false, 0);
}

static String build_string_with_separator (Allocator auto &allocator, const String_Builder &builder, char separator) {
  return build_string(allocator, builder, true, separator);
}

template <Allocator Alloc_Type>
static String concat_string (Alloc_Type &allocator, Printable auto &&... args) {
  String_Builder builder { allocator };
  defer { alloc_destroy<Alloc_Type>(builder); };

  const auto append = [&] (auto &&arg) {
    if constexpr (Convertible_To<decltype(arg), String_View>)
      builder += static_cast<String_View>(arg);
    else
      builder += to_string(arg, allocator);
  };

  (append(args), ...);

  return build_string(allocator, builder);
}

}
