
#define FILE_SYSTEM_HPP_IMPL

#include "anyfin/platform/file_system.hpp"

namespace Fin::Platform {

constexpr char get_path_separator() { return '\\'; }

constexpr Core::String_View get_static_library_extension() { return "lib"; }
constexpr Core::String_View get_shared_library_extension() { return "dll"; }
constexpr Core::String_View get_executable_extension()     { return "exe"; }
constexpr Core::String_View get_object_extension()         { return "obj"; }

static Result<void> create_resource (const File_Path &path, const Resource_Type resource_type, const Core::Bit_Mask<File_System_Flags> flags) {
  switch (resource_type) {
    case Resource_Type::File: {
      Core::trap("Unimplemented");
    }
    case Resource_Type::Directory: {
      if (CreateDirectory(path, NULL))            return Core::Ok();
      if (GetLastError() == ERROR_ALREADY_EXISTS) return Core::Ok();

      return Core::Error(get_system_error()); 
    }
  }
}

static Result<bool> check_resource_exists (const File_Path &path, Resource_Type resource_type) {
  const DWORD attributes = GetFileAttributes(path.value);
  if (attributes == INVALID_FILE_ATTRIBUTES) return Core::Error(get_system_error());

  switch (resource_type) {
    case Resource_Type::File:      return Core::Ok(!(attributes  & FILE_ATTRIBUTE_DIRECTORY));
    case Resource_Type::Directory: return Core::Ok(!!(attributes & FILE_ATTRIBUTE_DIRECTORY));
  }
}

static Result<void> delete_resource (const File_Path &path, Resource_Type resource_type) {
  switch (resource_type) {
    case Resource_Type::File: {
      Core::trap("Unimplemented");
    }
    case Resource_Type::Directory: {
      if (RemoveDirectory(path.value)) return Core::Ok();

      const auto error_code = GetLastError();

      if (error_code == ERROR_FILE_NOT_FOUND) return Core::Ok();

      if (error_code == ERROR_DIR_NOT_EMPTY) {
        Core::trap("Unimplemented");
        // Core::Scope_Allocator local { allocator };
        // return delete_directory_recursive(local, path);
      }

      return Core::Error(get_system_error());
    }
  }
}

static Result<Core::Option<Core::String>> get_resource_name (Core::Allocator auto &allocator, const File_Path &path) {
  char buffer[MAX_PATH];

  char *file_name_offset = nullptr;
  const auto length = GetFullPathName(path, MAX_PATH, buffer, &file_name_offset);
  if (!length) return Core::Error(get_system_error());

  if (file_name_offset) {
    const auto name_length = (buffer + length) - file_name_offset;
    return Core::Ok(Option(Core::String::copy(file_name_offset, name_length, allocator)));
  }
  
  Core::trap("Unimplemented");
}

static Result<File_Path> get_absolute_path (Core::Allocator auto &allocator, const File_Path &path) {
  auto full_path_name_length = GetFullPathNameA(path, 0, nullptr, nullptr);

  auto buffer = reinterpret_cast<char *>(reserve_memory(allocator, full_path_name_length, alignof(char)));

  if (!GetFullPathNameA(path, MAX_PATH, buffer, nullptr)) {
    free_reservation(allocator, buffer);
    return Core::Error(get_system_error());
  }

  return Core::Ok(File_Path(allocator, buffer, full_path_name_length));
}

static Result<File_Path> get_working_directory_path (Core::Allocator auto &allocator) {
  Core::trap("Unimplemented");
}

static Result<Core::List<File_Path>> list_files (Core::Allocator auto &allocator, const File_Path &directory, const Core::String_View &extension, bool recursive) {
  Core::trap("Unimplemented");
}

static Result<File> open_file (Core::Allocator auto &allocator, const File_Path &path, Core::Bit_Mask<File_System_Flags> flags) {
  Core::trap("Unimplemented");
}

static Result<void> close_file (File &file) {
  Core::trap("Unimplemented");
}

static Result<u64> get_file_size (const File &file) {
  Core::trap("Unimplemented");
}

static Result<u64> get_file_id (const File &file) {
  Core::trap("Unimplemented");
}

static Result<void> write_buffer_to_file (File &file, const Core::Iterable<const u8> auto &bytes) {
  Core::trap("Unimplemented");
}

static void reset_file_cursor (File &file) {
  Core::trap("Unimplemented");
}

static Result<u64> get_last_update_timestamp (const File &file) {
  Core::trap("Unimplemented");
}

static Result<File_Mapping> map_file_into_memory (const File &file) {
  Core::trap("Unimplemented");
}

static Result<void> unmap_file (File_Mapping &mapping) {
  Core::trap("Unimplemented");
}

}
