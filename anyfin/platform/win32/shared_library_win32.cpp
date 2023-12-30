
#include "anyfin/platform/shared_library.hpp"

#include "anyfin/platform/win32/base_win32.hpp"

namespace Fin::Platform {

Core::Result<System_Error, Shared_Library *> load_shared_library (const File_Path &library_file_path) {
  auto handle = LoadLibrary(library_file_path->value);
  if (handle == nullptr) return get_system_error(library_file_path->value);

  *library = reinterpret_cast<Shared_Library *>(handle);

  return Success;
}

Core::Result<System_Error, void> unload_library (Shared_Library &library);
void unload_library (Shared_Library *library) {
  FreeLibrary((HMODULE) library);
}

Core::Result<System_Error, void *> lookup_symbol (const Shared_Library &library, const Core::String_View &symbol_name);
void* load_symbol_from_library (const Shared_Library *library, const char *symbol_name) {
  return (void*) GetProcAddress((HMODULE) library, symbol_name);
}

}
