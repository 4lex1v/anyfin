
#define FILE_SYSTEM_HPP_IMPL

#include "anyfin/core/arena.hpp"
#include "anyfin/core/option.hpp"
#include "anyfin/core/slice.hpp"
#include "anyfin/core/strings.hpp"
#include "anyfin/core/meta.hpp"

#include "anyfin/platform/file_system.hpp"

namespace Fin::Platform {

constexpr char get_path_separator() { return '\\'; }

constexpr Core::String_View get_static_library_extension() { return "lib"; }
constexpr Core::String_View get_shared_library_extension() { return "dll"; }
constexpr Core::String_View get_executable_extension()     { return "exe"; }
constexpr Core::String_View get_object_extension()         { return "obj"; }

static Result<void> create_resource (File_Path_View path, const Resource_Type resource_type, const Core::Bit_Mask<File_System_Flags> flags) {
  switch (resource_type) {
    case Resource_Type::File: {
      using enum File_System_Flags;
      
      auto access  = GENERIC_READ    | ((flags & Write_Access) ? GENERIC_WRITE    : 0);
      auto sharing = FILE_SHARE_READ | ((flags & Shared_Write) ? FILE_SHARE_WRITE : 0); 

      auto handle = CreateFile(path.value, access, sharing, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
      if (handle == INVALID_HANDLE_VALUE) return get_system_error();

      return Core::Ok();
    }
    case Resource_Type::Directory: {
      if (CreateDirectory(path, NULL))            return Core::Ok();
      if (GetLastError() == ERROR_ALREADY_EXISTS) return Core::Ok();

      return Core::Error(get_system_error()); 
    }
  }
}

static Result<bool> check_resource_exists (File_Path_View path, Resource_Type resource_type) {
  const DWORD attributes = GetFileAttributes(path.value);
  if (attributes == INVALID_FILE_ATTRIBUTES) return Core::Error(get_system_error());

  switch (resource_type) {
    case Resource_Type::File:      return Core::Ok(!(attributes  & FILE_ATTRIBUTE_DIRECTORY));
    case Resource_Type::Directory: return Core::Ok(!!(attributes & FILE_ATTRIBUTE_DIRECTORY));
  }
}

static Result<void> delete_resource (File_Path_View path, Resource_Type resource_type) {
  switch (resource_type) {
    case Resource_Type::File: {
      if (!DeleteFile(path.value)) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) return Core::Ok();
        return get_system_error();
      }

      return Core::Ok();
    }
    case Resource_Type::Directory: {
      if (RemoveDirectory(path.value)) return Core::Ok();

      auto error_code = GetLastError();
      if (error_code == ERROR_FILE_NOT_FOUND) return Core::Ok();
      if (error_code == ERROR_DIR_NOT_EMPTY)  {
        Core::Local_Arena<2048> arena;

        auto delete_recursive = [] (this auto self, Core::Memory_Arena &allocator, File_Path_View path) -> Result<void> {
          auto directory_search_query = format_string(allocator, "%\\*", path);
  
          WIN32_FIND_DATA data;
          auto search_handle = FindFirstFile(directory_search_query, &data);
          if (search_handle == INVALID_HANDLE_VALUE) return Core::Error(get_system_error());
          defer { FindClose(search_handle); };

          do {
            auto local = allocator;

            auto file_name = Core::String_View(data.cFileName);
            auto sub_path  = format_string(local, "%\\%", path, file_name);

            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
              if (Core::compare_strings(file_name, ".") || Core::compare_strings(file_name, "..")) continue;
              fin_check(self(local, sub_path));
            }
            else fin_check(delete_file(sub_path));
          } while (FindNextFile(search_handle, &data));

          if (!RemoveDirectory(path.value)) return Core::Error(get_system_error());

          return Core::Ok();
        };

        return delete_recursive(arena, path); 
      }

      return get_system_error();
    }
  }
}

static Result<Core::Option<Core::String>> get_resource_name (Core::Allocator auto &allocator, File_Path_View path) {
  char buffer[MAX_PATH];

  char *file_name_offset = nullptr;
  const auto length = GetFullPathName(path, MAX_PATH, buffer, &file_name_offset);
  if (!length) return get_system_error();

  if (file_name_offset) {
    const auto name_length = (buffer + length) - file_name_offset;
    return Core::Option(Core::String::copy(allocator, file_name_offset, name_length));
  }
  
  Core::trap("Unimplemented");
}

static Result<File_Path> get_absolute_path (Core::Allocator auto &allocator, File_Path_View path) {
  auto full_path_name_length = GetFullPathName(path, 0, nullptr, nullptr);

  auto buffer = reserve(allocator, full_path_name_length, alignof(char));

  if (!GetFullPathName(path, full_path_name_length, buffer, nullptr)) {
    free(allocator, buffer);
    return Core::Error(get_system_error());
  }

  return Core::Ok(File_Path(allocator, buffer, full_path_name_length - 1));
}

static Result<Core::Option<File_Path>> get_parent_folder_path (Core::Allocator auto &allocator, File_Path_View _path) {
  auto [tag, error, path] = get_absolute_path(allocator, _path);
  if (!tag) return Core::move(error);

  for (usize idx = path.length; idx > 0; idx--) {
    if (path.value[idx] == '/' || path.value[idx] == '\\') {
      return Core::Option(Core::String::copy(allocator, path.value, idx));
    }
  }
  
  return Core::Option<File_Path> {};
}

static Result<File_Path> get_working_directory (Core::Allocator auto &allocator) {
  auto buffer_size = GetCurrentDirectory(0, nullptr);
  if (buffer_size == 0) return Core::Error(get_system_error());
  
  auto buffer = reserve<char>(allocator, buffer_size);

  auto path_length = GetCurrentDirectory(buffer_size, buffer);
  if (!path_length) return get_system_error();

  return File_Path(allocator, buffer, path_length);
}

static Result<void> set_working_directory (File_Path_View path) {
  if (!SetCurrentDirectory(path.value)) return get_system_error();
  return Core::Ok();
}

static Result<Core::List<File_Path>> list_files (Core::Allocator auto &allocator, File_Path_View directory, Core::String_View extension, bool recursive) {
  Core::List<File_Path>   file_list { allocator };
  Core::Local_Arena<2048> arena;

  auto list_recursive = [extension, recursive, &file_list] (this auto self, Core::Memory_Arena &arena, File_Path_View directory) -> Result<void> {
    WIN32_FIND_DATAA data;

    auto query = format_string(arena, "%\\*", directory);

    auto search_handle = FindFirstFile(query, &data);
    if (search_handle == INVALID_HANDLE_VALUE) return Core::Error(get_system_error());
    defer { FindClose(search_handle); };

    do {
      auto local = arena;
      
      const auto file_name = String_View(data.cFileName);

      if (compare_strings(file_name, ".") ||
          compare_strings(file_name, "..")) continue;

      if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (recursive) {
          auto subfolder = format_string(local, "%\\%", directory, Core::String_View(data.cFileName));
          fin_check(self(local, subfolder));
        }
      }
      else {
        if (!ends_with(file_name, extension)) continue;
          
        auto file_path = make_file_path(local, "%\\%", directory, Core::String_View(data.cFileName));
        if (!file_list.contains([&file_path] (auto &it) { return compare_strings(it, file_path); }))
          list_push(file_list, move(file_path));
      }

    } while (FindNextFileA(search_handle, &data) != 0);

    return Core::Ok();
  };

  fin_check(list_recursive(arena, directory));

  return Core::Ok(file_list);
}

static Result<void> copy_directory (File_Path_View from, File_Path_View to) {
  Core::Local_Arena<2048> arena;

  auto copy_recursive = [] (this auto self, Core::Memory_Arena &arena, File_Path_View from, File_Path_View to) -> Result<void> {
    WIN32_FIND_DATA find_file_data;

    auto search_query = format_string(arena, "%\\*", from);

    auto search_handle = FindFirstFile(search_query.value, &find_file_data);
    if (search_handle == INVALID_HANDLE_VALUE) return Core::Error(get_system_error());
    defer { FindClose(search_handle); };

    do {
      auto scoped = arena;

      auto file_to_move = format_string(scoped, "%\\%", from, find_file_data.cFileName);
      auto destination  = format_string(scoped, "%\\%", to,   find_file_data.cFileName);

      if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (compare_strings(Core::String_View(find_file_data.cFileName), ".") == 0 ||
            compare_strings(Core::String_View(find_file_data.cFileName), "..") == 0) continue;

        if (!CreateDirectory(destination, nullptr)) return get_system_error();

        fin_check(self(scoped, file_to_move, destination));
      }
      else {
        if (!CopyFile(file_to_move.value, destination.value, FALSE)) return get_system_error();
      }
    } while (FindNextFile(search_handle, &find_file_data) != 0);

    return Core::Ok();
  };

  fin_check(create_directory(to));

  return copy_recursive(arena, from, to);
}

static Result<File> open_file (File_Path &&path, Core::Bit_Mask<File_System_Flags> flags) {
  using enum File_System_Flags;

  auto access  = GENERIC_READ    | ((flags & Write_Access) ? GENERIC_WRITE    : 0);
  auto sharing = FILE_SHARE_READ | ((flags & Shared_Write) ? FILE_SHARE_WRITE : 0); 
  auto status  = (flags & Create_Missing) ? OPEN_ALWAYS : OPEN_EXISTING;
  
  auto handle = CreateFile(path.value, access, sharing, NULL, status, FILE_ATTRIBUTE_NORMAL, NULL);
  if (handle == INVALID_HANDLE_VALUE) return get_system_error();

  return File { handle, move(path) };
}

static Result<void> close_file (File &file) {
  if (CloseHandle(file.handle) == 0) return get_system_error();
  file.handle = nullptr;
  return Core::Ok();
}

static Result<u64> get_file_size (const File &file) {
  LARGE_INTEGER file_size;
  if (GetFileSizeEx(file.handle, &file_size) == false) return get_system_error();

  return file_size.QuadPart;
}

static Result<u64> get_file_id (const File &file) {
  FILE_ID_INFO id_info;
  if (!GetFileInformationByHandleEx(file.handle, FileIdInfo, &id_info, sizeof(id_info)))
    return get_system_error();

  return Core::Ok(*reinterpret_cast<u64 *>(id_info.FileId.Identifier));
}

static Result<void> write_buffer_to_file (File &file, Core::String_View bytes) {
  DWORD total_bytes_written = 0;
  while (total_bytes_written < bytes.length) {
    DWORD bytes_written = 0;
    if (!WriteFile(file.handle, bytes.value + total_bytes_written, 
                   bytes.length - total_bytes_written, &bytes_written, nullptr)) {
      return get_system_error();
    }

    if (bytes_written == 0) {
      // No more bytes were written, could be a device error or a full disk.
      return get_system_error();  // or a custom error indicating partial write
    }

    total_bytes_written += bytes_written;
  }

  return Core::Ok();
}

static Result<void> reset_file_cursor (File &file) {
  if (SetFilePointer(file.handle, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    return get_system_error();

  return Core::Ok();
}

static Result<u64> get_last_update_timestamp (const File &file) {
  FILETIME last_update = {};
  if (!GetFileTime(file.handle, 0, 0, &last_update)) return get_system_error();

  ULARGE_INTEGER value;
  value.HighPart = last_update.dwHighDateTime;
  value.LowPart  = last_update.dwLowDateTime;

  return static_cast<u64>(value.QuadPart);
}

static Result<File_Mapping> map_file_into_memory (const File &file) {
  if (!get_file_size(file)) return File_Mapping {};
  
  auto handle = CreateFileMapping(file.handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
  if (handle == nullptr) return get_system_error();

  auto memory = MapViewOfFile(handle, FILE_MAP_READ, 0, 0, 0);
  if (memory == nullptr) { CloseHandle(handle); return get_system_error(); }

  auto [has_failed, error, mapping_size] = get_file_size(file);
  if (!has_failed) {
    UnmapViewOfFile(memory);
    CloseHandle(handle);
    return Core::Error(error);
  }

  return File_Mapping {
    .handle = handle,
    .memory = reinterpret_cast<char *>(memory),
    .size   = mapping_size
  };
}

static Result<void> unmap_file (File_Mapping &mapping) {
  // Windows doesn't allow mapping empty files. I'm not treating this as an error, thus
  // it should be handled gracefully here as well.
  if (!mapping.handle) return Core::Ok();
  
  if (!UnmapViewOfFile(mapping.memory))  return get_system_error();
  if (!CloseHandle(mapping.handle))      return get_system_error();

  return Core::Ok();
}

}
