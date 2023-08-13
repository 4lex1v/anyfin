
#pragma once

#include "anyfin/base.hpp"
#include "anyfin/core/meta.hpp"
#include "anyfin/core/prelude.hpp"

template <typename T> struct Bit_Mask;
struct Memory_Arena;
struct String;

struct File_Path {
  const char *value  = nullptr;
  usize       length = 0;
};

Result<File_Path> make_file_path_from_array (Memory_Arena &arena, const String *segments, usize count);

template <typename... Segment>
Result<File_Path> make_file_path (Memory_Arena &arena, Segment&&... segments) {
  auto to_string = [] <typename S> (const S segment) -> String {
    if constexpr (same_types<S, File_Path>)
      return String { segment.value, segment.length };
    return segment;
  };

  String string_segments [] { to_string(segments)... };

  return make_file_path_from_array(arena, string_segments, sizeof...(Segment));
}

struct File {
  struct Handle;

  Handle    *handle;
  File_Path  path;
};

enum struct Open_File_Flags: u32 {
  Write_Access   = flag(0),
  Shared_Write   = flag(1),
  Create_Missing = flag(2),
};

Result<File> open_file (const File_Path &path, Bit_Mask<Open_File_Flags> flags = {});
