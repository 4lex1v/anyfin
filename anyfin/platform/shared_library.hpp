
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/result.hpp"

#include "anyfin/platform/files.hpp"
#include "anyfin/platform/platform.hpp"

namespace Fin {
namespace Platform {

struct Shared_Library;

Core::Result<System_Error, Shared_Library *> load_shared_library (const File_Path &library_file_path);

Core::Result<System_Error, void> unload_library (Shared_Library &library);

Core::Result<System_Error, void *> lookup_symbol (const Shared_Library &library, const Core::String_View &symbol_name);

template <typename T>
static inline Core::Result<System_Error, T *> lookup_symbol (const Shared_Library &library, const Core::String_View &symbol_name) {
  return lookup_symbol(library, symbol_name).as<T *>();
}

}
}

