
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/bit_mask.hpp"
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
static File_Path make_file_path (Core::Allocator auto &allocator, const Core::Iterable<Core::String_View> auto &segments) {
  usize reservation_size = 0;
  // Segments will be separated with a platform-dependent path separator and be null-terminated.
  for (auto &s: segments) if (s) reservation_size += s.length + 1; 

  auto buffer = reinterpret_cast<char *>(reserve_memory(allocator, reservation_size));
  if (!buffer) Core::trap("Provided allocator is out of available memory");

  auto cursor = buffer;
  for (auto &segment: segments) {
    memcpy(cursor, segment.value, segment.length);
    cursor[segment.length] = get_path_separator();
    cursor += segment.length + 1;
  }

  buffer[reservation_size - 1] = '\0';

  return File_Path(allocator, buffer, reservation_size - 1);
}

/*
  Alternative API for a more convenient invocation of `make_file_path` using multiple segments.
  Each segment must be representable as a string.
 */
static File_Path make_file_path (Core::Allocator auto &allocator, Core::Convertible_To<Core::String_View> auto&&... segments) {
  Core::String_View string_segments [] { segments... };
  return make_file_path(allocator, string_segments);
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
static Result<void> create_resource (const File_Path &path, Resource_Type resource_type, Core::Bit_Mask<File_System_Flags> flags = {});

static Result<void> create_file (const File_Path &path, Core::Bit_Mask<File_System_Flags> flags = {}) {
  return create_resource(path, Resource_Type::File, flags);
}

static Result<void> create_directory (const File_Path &path, Core::Bit_Mask<File_System_Flags> flags = {}) {
  return create_resource(path, Resource_Type::Directory, flags);
}

/*
  Check if the file pointed by the provided path exists on the file system.
  Returns true or false if the resource exists of not.
 */
static Result<bool> check_resource_exists (const File_Path &path, Resource_Type resource_type);

/*
  Check if the provided path corresponds to an existing file on the file system.
 */
static Result<bool> check_file_exists (const File_Path &path) {
  return check_resource_exists(path, Resource_Type::File);
}

/*
  Check if the provided path corresponds to an existing directory on the file system.
 */
static Result<bool> check_directory_exists (const File_Path &path) {
  return check_resource_exists(path, Resource_Type::Directory);
}

/*
  Delete resource pointed by the path.
  If the resource doesn't exist, just returns to the caller.
  If the resource does exist, attempt to delete it.
  If the resource is a directory that has content, it will be recursively deleted.
 */
static Result<void> delete_resource (const File_Path &path, Resource_Type resource_type);

/*
  Attempts to remove a file from the file system that corresponds to the given path.
 */
static Result<void> delete_file (const File_Path &path) {
  return delete_resource(path, Resource_Type::File);
}

/*
  Attempts to remove a directory with all its content from the file system.
 */
static Result<void> delete_directory (const File_Path &path) {
  return delete_resource(path, Resource_Type::Directory);
}

/*
  Extracts the resource (directory or file) name from the path, regardless if the actual
  resource exists on the file system or not. If it's a file and has an extension, the extension
  would be included.
 */
static Result<Core::Option<Core::String>> get_resource_name (Core::Allocator auto &allocator, const File_Path &path);

static Result<File_Path> get_absolute_path (Core::Allocator auto &allocator, const File_Path &path);

static Result<Core::Option<File_Path>> get_parent_folder_path (Core::Allocator auto &allocator, const File_Path &file);

static Result<File_Path> get_working_directory (Core::Allocator auto &allocator);

static Result<void> set_working_directory (const File_Path &path);

static Result<Core::List<File_Path>> list_files (Core::Allocator auto &allocator, const File_Path &directory, Core::String_View extension = {}, bool recursive = false);

static Result<void> copy_directory (const File_Path &from, const File_Path &to);

struct File {
  void *handle;
  File_Path path;
};

static Result<File> open_file (File_Path &&path, Core::Bit_Mask<File_System_Flags> flags = {});

static Result<void> close_file (File &file);

static Result<u64> get_file_size (const File &file); 

// TODO: Fix this. On Windows the unique id is at least 128 bytes?
static Result<u64> get_file_id (const File &file);

static Result<void> write_buffer_to_file (File &file, const Core::Slice<const u8> &bytes);

static Result<void> reset_file_cursor (File &file);

static Result<u64> get_last_update_timestamp (const File &file);

struct File_Mapping {
  void *handle;
  
  char *memory;
  usize size;
};

static Result<File_Mapping> map_file_into_memory (const File &file);

static Result<void> unmap_file (File_Mapping &mapping);

}

#ifndef FILE_SYSTEM_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "file_system_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
