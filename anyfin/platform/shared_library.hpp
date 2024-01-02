
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/result.hpp"

#include "anyfin/platform/file_system.hpp"
#include "anyfin/platform/platform.hpp"

namespace Fin::Platform {

struct Shared_Library;

static Result<Shared_Library *> load_shared_library (const File_Path &library_file_path);

static Result<void> unload_library (Shared_Library &library);

template <typename T>
static Result<T *> lookup_symbol (const Shared_Library &library, const Core::String_View &symbol_name);

}

#ifndef SHARED_LIBRARY_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "shared_library_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
