
#include "anyfin/core/option.hpp"
#include "anyfin/core/result.hpp"
#include "anyfin/core/trap.hpp"

#include "anyfin/platform/files.hpp"
#include "anyfin/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include "anyfin/platform/win32/base_win32.hpp"

namespace Fin::Platform {

Result<File> open_file (const File_Path &path, const Core::Bit_Mask<Open_File_Flags> flags) {
  using enum Open_File_Flags;

  auto access  = GENERIC_READ    | ((flags & Write_Access) ? GENERIC_WRITE    : 0);
  auto sharing = FILE_SHARE_READ | ((flags & Shared_Write) ? FILE_SHARE_WRITE : 0); 
  auto status  = (flags & Create_Missing) ? OPEN_ALWAYS : OPEN_EXISTING;
  
  auto handle = CreateFile(path.value, access, sharing, NULL, status, FILE_ATTRIBUTE_NORMAL, NULL);
  if (handle == INVALID_HANDLE_VALUE) {
    if (status == OPEN_EXISTING && GetLastError() == ERROR_FILE_NOT_FOUND)
      return Core::Error(System_Error { .error_code = ERROR_FILE_NOT_FOUND });

    return Core::Error(get_system_error());
  }

  //return Core::Ok(File(reinterpret_cast<File::Handle *>(handle), path));
  return Core::Ok(File(reinterpret_cast<File::Handle *>(handle)));
}

Result<File_Path> get_working_directory_path (Core::Allocator_View allocator) {
  auto buffer_size = GetCurrentDirectory(0, nullptr);
  if (buffer_size == 0) return Core::Error(get_system_error());
  
  auto buffer = reinterpret_cast<char *>(reserve_memory(allocator, buffer_size));

  auto path_length = GetCurrentDirectory(buffer_size, buffer);
  if (!path_length) return Core::Error(get_system_error());

  return Core::Ok(File_Path(buffer, path_length, allocator));
}

static Result<void> create_directory_recursive_inner (char *path, const usize length) {
  auto attributes = GetFileAttributes(path);

  // Something went wrong
  if (attributes == INVALID_FILE_ATTRIBUTES) return Core::Error(get_system_error());

  // Directory already exists
  if (attributes & FILE_ATTRIBUTE_DIRECTORY) return Core::Ok();

  auto separator = path + length;
  while (separator > path) {
    if (*separator == '\\') break;
    separator -= 1;
  }

  // File path base is reached
  if (separator == path) return Core::Ok();

  *separator = '\0';
  auto [tag, error] = create_directory_recursive_inner(path, separator - path);
  if (!tag) return Core::Error(error);
  *separator = '\\';

  if (!CreateDirectory(path, nullptr)) {
    if (GetLastError() == ERROR_ALREADY_EXISTS) return Core::Ok();
    return Core::Error(get_system_error());
  }

  return Core::Ok();
}

Result<void> create_directory_recursive (const File_Path &path) {
  char path_copy[path.length + 1];
  memcpy(path_copy, path.value, path.length);
  path_copy[path.length] = '\0';

  return create_directory_recursive_inner(path_copy, path.length);
}

Result<void> write_buffer_to_file (File &file, const Core::Slice<const u8> &data) {
  DWORD bytes_written = 0;
  if (!WriteFile(file.handle, data.elements, data.count, &bytes_written, nullptr))
    return Core::Error(get_system_error());

  if (bytes_written != data.count) 
    return Core::Error(get_system_error());

  return Core::Ok();
}

Result<void> close_file (File &file) {
  if (CloseHandle(file.handle) == 0) return Core::Error(get_system_error());

  file.handle = nullptr;

  return Core::Ok();
}

static inline Result<void> delete_directory_recursive (Core::Allocator auto &allocator, const File_Path &path) {
  auto directory_search_query = format_string(allocator, "%\\*", path);
  
  WIN32_FIND_DATA data;
  auto search_handle = FindFirstFile(directory_search_query, &data);
  if (search_handle == INVALID_HANDLE_VALUE) return Core::Error(get_system_error());
  defer { FindClose(search_handle); };

  do {
    if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      if ((strcmp(data.cFileName, ".") != 0) &&
          (strcmp(data.cFileName, "..") != 0)) {
        auto subpath = format_string(allocator, "%\\%", path, Core::String_View(data.cFileName));
        if (auto result = delete_directory_recursive(allocator, subpath); !result) return result;
      }
    }
    else {
      auto subpath = format_string(allocator, "%\\%", path, Core::String_View(data.cFileName));
      if (auto result = delete_file(subpath); !result) return result;
    }
  } while (FindNextFile(search_handle, &data));

  if (!RemoveDirectory(path.value)) return Core::Error(get_system_error());

  return Core::Ok();
}

static Result<void> list_directory_files_step (const File_Path &directory, const Core::String_View &extension, bool recurse, Core::List<File_Path> &file_list) {
  auto query = format_string(allocator, "%\\*", directory);

  WIN32_FIND_DATAA data;
  HANDLE search_handle;
  defer { FindClose(search_handle); };

  if ((search_handle = FindFirstFileA(query, &data)) != INVALID_HANDLE_VALUE) {
    do {
      if (strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0) {
        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          if (recurse) {
            auto subfolder = format_string(allocator, "%\\%", directory, Core::String_View(data.cFileName));
            check(list_directory_files_step(subfolder, extension, recurse, file_list));
          }
        } else {
          if (ends_with(Core::String_View(data.cFileName), extension)) continue;
          
          // The reservation size includes the path separator and terminating null
          auto reservation_size = directory.length + strlen(data.cFileName) + 2;

          auto file_path = format_string(allocator, "%\\%s", directory, Core::String_View(data.cFileName));
          if (!file_list.contains([&file_path] (auto it) { return compare_strings(it, file_path); }))
            list_push(file_list, move(file_path));
        }
      }
    } while (FindNextFileA(search_handle, &data) != 0);
  }

  return Core::Ok();
}

Result<Core::List<File_Path>> list_files_in_directory (const Core::Allocator &allocator, const File_Path &directory,
                                                       const Core::String_View &extension, bool recursive) {
  Core::List<File_Path> file_list { allocator };

  auto result = list_directory_files_step(directory, extension, recursive, file_list);
  if (!result) return Core::Error(result.status);

  return Core::Ok(file_list);
  
  // char path[MAX_PATH];
  // snprintf(path, MAX_PATH, );
  // format_string(allocator, "%.*s\\*", (int)absolute_file_path.length, absolute_file_path.value);

  // WIN32_FIND_DATAA data;
  // HANDLE search_handle;
  // defer { FindClose(search_handle); };

  // auto has_extension = [&extension] (const WIN32_FIND_DATAA &data) -> bool {
  //   auto file_name = Core::String_View(data.cFileName);
  //   return compare_strings(file_name, extension);
  // };

  // auto check_if_path_already_included = [] (const List<File_Path> *paths, const File_Path *path) -> bool {
  //   for (auto &included_path: *paths) {
  //     if (compare_strings(included_path, *path)) return true;
  //   }

  //   return false;
  // };

  // if ((search_handle = FindFirstFileA(path, &data)) != INVALID_HANDLE_VALUE) {
  //   do {
  //     if (strcmp(data.cFileName, ".") != 0 && strcmp(data.cFileName, "..") != 0) {
  //       if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
  //         if (recursive) {
  //           snprintf(path, MAX_PATH, "%.*s\\%s", (int)absolute_file_path.length, absolute_file_path.value, data.cFileName);
  //           list_files_in_directory(arena, list, path, extension, recursive);
  //         }
  //       } else {
  //         if (!has_extension(&data, extension)) continue;
          
  //         // The reservation size includes the path separator and terminating null
  //         auto reservation_size = absolute_file_path.length + strlen(data.cFileName) + 2;
  //         auto file_path = reserve_array<char>(arena, reservation_size);

  //         snprintf(file_path, reservation_size, "%.*s\\%s", (int)absolute_file_path.length, absolute_file_path.value, data.cFileName);

  //         if (auto new_path = File_Path(file_path);
  //             !check_if_path_already_included(list, &new_path))
  //           add(arena, list, new_path);
  //       }
  //     }
  //   } while (FindNextFileA(search_handle, &data) != 0);
  // }
}

Result<u64> get_file_size (const File &file) {
  LARGE_INTEGER file_size;
  if (GetFileSizeEx(file.handle, &file_size) == false)
    return Core::Error(get_system_error());

  return Core::Ok<u64>(file_size.QuadPart);
}

void reset_file_cursor (File &file) {
  SetFilePointer(file.handle, 0, nullptr, FILE_BEGIN);
}

Result<u64> get_file_id (const File &file) {
  FILE_ID_INFO id_info;
  if (!GetFileInformationByHandleEx(file.handle, FileIdInfo, &id_info, sizeof(id_info)))
    return Core::Error(get_system_error());

  return Core::Ok(*reinterpret_cast<u64 *>(id_info.FileId.Identifier));
}

Result<Core::Option<File_Path>> get_parent_folder_path (const File &file) {
  auto [tag, error, path] = get_absolute_path(file.path);
  if (!tag) return Core::move(error);

  for (usize idx = path.length; idx > 0; idx--) {
    if (path.value[idx] == '/' || path.value[idx] == '\\') {
      return Core::Option(File_Path(path.substring(0, idx)));
    }
  }
  
  return Core::Option<File_Path> {};
}

}
