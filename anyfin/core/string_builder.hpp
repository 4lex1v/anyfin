
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/strings.hpp"

struct Memory_Arena;

struct String_Builder {
  Memory_Arena *arena;
  List<String> sections = {};

  usize length = 0;

  String_Builder (Memory_Arena *_arena): arena { _arena } {}

  void add (String value) {
    if (value.length == 0) return;

    ::add(this->arena, &this->sections, value);
    this->length += value.length;
  }

  void add (const List<String> &list) {
    for (auto value: list) add(value);
  }

  void operator += (const String &value)        { add(value); }
  void operator += (const List<String> &values) { add(values); }
  
  template <String_Convertible S>
  void operator += (S &&value) { add(make_string(this->arena, value)); }
};
