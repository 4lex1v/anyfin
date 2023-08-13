
#pragma once

#include "anyfin/base.hpp"

template <typename T> struct Result;
struct Memory_Arena;
struct File;

struct Font_Atlas {
  struct Context;

  Context *context;

  const File &font_file;

  usize width;
  usize height;
}

Result<Font_Atlas> create_font_atlas (Memory_Arena &arena, const File &font_file, usize raster_width, usize raster_height);



