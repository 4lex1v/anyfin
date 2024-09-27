#define FIN_SHARED_LIBRARY_HPP_IMPL

// No need to include "anyfin/win32.hpp"

#include "anyfin/shared_library.hpp"

#include <dlfcn.h>
#include <errno.h>
#include <cstring>

namespace Fin {

static Sys_Result<Shared_Library *> load_shared_library (const File_Path &library_file_path) {
  void *handle = dlopen(library_file_path.value, RTLD_LAZY);
  if (handle == nullptr) {
    const char *error_message = dlerror();
    return Error(System_Error { String(error_message), 0 });
  }

  return reinterpret_cast<Shared_Library *>(handle);
}

static Sys_Result<void> unload_library (Shared_Library &library) {
  if (dlclose(reinterpret_cast<void *>(&library)) != 0) {
    const char *error_message = dlerror();
    return Error(System_Error { String(error_message), 0 });
  }

  return Ok();
}

template <typename T>
static Sys_Result<T *> lookup_symbol (const Shared_Library &library, const String &symbol_name) {
  void *symbol = dlsym(reinterpret_cast<void *>(&library), symbol_name.value);
  if (!symbol) {
    const char *error_message = dlerror();
    return Error(System_Error { String(error_message), 0 });
  }

  return reinterpret_cast<T *>(symbol);
}

}
