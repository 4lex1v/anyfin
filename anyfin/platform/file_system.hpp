
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/arrays.hpp"
#include "anyfin/core/bit_mask.hpp"
#include "anyfin/core/memory.hpp"
#include "anyfin/core/meta.hpp"
#include "anyfin/core/strings.hpp"
#include "anyfin/core/slice.hpp"

#include "anyfin/platform/platform.hpp"

namespace Fin::Platform {

constexpr char get_path_separator();

constexpr Core::String_View get_static_library_extension();
constexpr Core::String_View get_shared_library_extension();
constexpr Core::String_View get_executable_extension();
constexpr Core::String_View get_object_extension();

using File_Path      = Core::String;
using File_Path_View = Core::String_View;

/*
  Construct a platform-dependent file path.
  Path separator is platform-dependent, i.e for Windows it's \, while for Unix systems - /.
 */
static File_Path make_file_path (Core::Allocator auto &allocator, Core::Convertible_To<Core::String_View> auto&&... segments) {
  char *buffer = nullptr;
  usize length = 0;

  const auto push_segment = [&] (Core::String_View segment) {
    /*
      This would allow using conditional parts in the file path construction.
     */
    if (is_empty(segment)) return;
    
    auto reservation_size = segment.length + 1;

    auto memory = grow(allocator, &buffer, length, reservation_size, buffer != nullptr, alignof(char));

    Core::copy_memory(memory, segment.value, segment.length);
    memory[segment.length] = get_path_separator();

    length += reservation_size;
  };

  (push_segment(segments), ...);

  /*
    The last path separator would be replace with a 0 to terminate the string with a null-term.
    but the length should also be decremented to not include it.
  */
  buffer[--length] = '\0';

#ifdef PLATFORM_WIN32
  for (usize idx = 0; idx < length; idx++) {
    if (buffer[idx] == '/') buffer[idx] = '\\';
  }
#endif

  return File_Path(allocator, buffer, length);
}

enum struct Resource_Type { File, Directory };

enum struct File_System_Flags: u64 {
  Write_Access   = flag(0),
  Shared_Write   = flag(1),
  Create_Missing = flag(2),
  Force          = flag(3),
};

/*
  Create a resource of a specified type on the file system under the specified path.
 */
static Result<void> create_resource (File_Path_View path, Resource_Type resource_type, Core::Bit_Mask<File_System_Flags> flags = {});

static Result<void> create_file (File_Path_View path, Core::Bit_Mask<File_System_Flags> flags = {}) {
  return create_resource(path, Resource_Type::File, flags);
}

static void create_file (Core::Allocator auto &allocator, File_Path_View path, Core::Bit_Mask<File_System_Flags> flags = {}) {
  create_file(path).expect([&] (auto error) -> Core::String_View {
    return concat_string(allocator, "ERROR: Couldn't create file ", path, " due to a system error: ", error, "\n");
  });
}

fin_forceinline
static Result<void> create_directory (File_Path_View path, Core::Bit_Mask<File_System_Flags> flags = {}) {
  return create_resource(path, Resource_Type::Directory, flags);
}

fin_forceinline
static void create_directory (Core::Allocator auto &allocator, File_Path_View path, Core::Bit_Mask<File_System_Flags> flags = {}) {
  create_resource(path, Resource_Type::Directory, flags).expect([&](auto error) -> Core::String_View {
    return concat_string(allocator, "ERROR: Couldn't create directory ", path, " due to a system error: ", error, "\n");
  });
}

/*
  Check if the file pointed by the provided path exists on the file system.
  Returns true or false if the resource exists of not.
 */
static Result<bool> check_resource_exists (File_Path_View path, Resource_Type resource_type) ;

/*
  Check if the provided path corresponds to an existing file on the file system.
 */
static Result<bool> check_file_exists (File_Path_View path) {
  return check_resource_exists(path, Resource_Type::File);
}

static bool check_file_exists (Core::Allocator auto &allocator, File_Path_View path) {
  return check_file_exists(path).take([&] (auto error) -> Core::String_View {
    return concat_string(allocator, "ERROR: Path ", path, " validation has failed due to a system error: ", error);
  });
}

/*
  Check if the provided path corresponds to an existing directory on the file system.
 */
static Result<bool> check_directory_exists (File_Path_View path) {
  return check_resource_exists(path, Resource_Type::Directory);
}

static bool check_directory_exists (Core::Allocator auto &allocator, File_Path_View path) {
  return check_directory_exists(path).take([&] (auto error) -> Core::String_View {
    return concat_string(allocator, "ERROR: Path ", path, " validation has failed due to a system error: ", error);
  });
}

/*
  Delete resource pointed by the path.
  If the resource doesn't exist, just returns to the caller.
  If the resource does exist, attempt to delete it.
  If the resource is a directory that has content, it will be recursively deleted.
 */
static Result<void> delete_resource (File_Path_View path, Resource_Type resource_type) ;

/*
  Attempts to remove a file from the file system that corresponds to the given path.
 */
static Result<void> delete_file (File_Path_View path) {
  return delete_resource(path, Resource_Type::File);
}

/*
  Attempts to remove a directory with all its content from the file system.
 */
static Result<void> delete_directory (File_Path_View path) {
  return delete_resource(path, Resource_Type::Directory);
}

/*
  Extracts the resource (directory or file) name from the path, regardless if the actual
  resource exists on the file system or not. If it's a file and has an extension, the extension
  would be included.
 */
static Result<Core::String> get_resource_name (Core::Allocator auto &allocator, File_Path_View path);

static Result<File_Path> get_absolute_path (Core::Allocator auto &allocator, File_Path_View path);

static Result<File_Path> get_folder_path (Core::Allocator auto &allocator, File_Path_View file);

static Result<File_Path> get_working_directory (Core::Allocator auto &allocator);

static Result<void> set_working_directory (File_Path_View path);

static Result<void> for_each_file (File_Path_View directory, Core::String_View extension, bool recursive, const Core::Invocable<bool, File_Path_View> auto &func);

static Result<Core::List<File_Path>> list_files (Core::Allocator auto &allocator, File_Path_View directory, Core::String_View extension = {}, bool recursive = false);

static Result<void> copy_directory (File_Path_View from, File_Path_View to);

struct File {
  void *handle;
  File_Path path;
};

static Result<File> open_file (File_Path &&path, Core::Bit_Mask<File_System_Flags> flags = {});

static File open_file (Core::Allocator auto &allocator, File_Path &&path, Core::Bit_Mask<File_System_Flags> flags = {}, Core::Callsite_Info callsite = {}) {
  Core::String_View path_ref { path };
  return open_file(Core::move(path), flags).take([&] (auto error) -> Core::String_View {
    return concat_string(allocator, callsite, ": ERROR: Couldn't open file ", path_ref, " due to a system error: ", error, "\n");
  });
}

static Result<void> close_file (File &file);

static Result<u64> get_file_size (const File &file); 

static Result<u64> get_file_id (const File &file);

static u64 get_file_id (Core::Allocator auto &allocator, const File &file) {
  return get_file_id(file).take([&] (auto error) -> Core::String_View {
    return concat_string(allocator, "ERROR: Dependencies scanner has failed to retrieve file's ", file.path, " id due to a system error: ", error);
  });
}

template <Core::Byte_Type T>
static Result<void> write_buffer_to_file (File &file, Core::Slice<T> bytes);

static Result<void> read_bytes_into_buffer (File &file, u8 *buffer, usize bytes_to_read);

static Result<Core::Array<u8>> get_file_content (Core::Allocator auto &allocator, File &file);

static Result<void> reset_file_cursor (File &file);

static Result<u64> get_last_update_timestamp (const File &file);

struct File_Mapping {
  void *handle;
  
  char *memory;
  usize size;
};

static Result<File_Mapping> map_file_into_memory (const File &file);

static File_Mapping map_file_into_memory (Core::Allocator auto &allocator, const File &file) {
  return map_file_into_memory(file).take([&] (auto error) -> Core::String_View {
    return concat_string(allocator, "ERROR: Failed to map file ", file.path, " into memory due to a system error: ", error);
  });
}

static Result<void> unmap_file (File_Mapping &mapping);

}

#ifndef FILE_SYSTEM_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "file_system_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
