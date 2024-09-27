
#define FIN_FILE_SYSTEM_HPP_IMPL

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#include <limits.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <libgen.h>

#include "anyfin/arena.hpp"
#include "anyfin/option.hpp"
#include "anyfin/strings.hpp"
#include "anyfin/meta.hpp"
#include "anyfin/defer.hpp"

#include "anyfin/file_system.hpp"

namespace Fin {

constexpr char get_path_separator() { return '/'; }

constexpr String get_static_library_extension() { return "a"; }
constexpr String get_shared_library_extension() { return "so"; }
constexpr String get_executable_extension()     { return ""; }
constexpr String get_object_extension()         { return "o"; }

static Sys_Result<void> create_resource (File_Path path, const Resource_Type resource_type, const Bit_Mask<File_System_Flags> flags) {
  switch (resource_type) {
    case Resource_Type::File: {
      using enum File_System_Flags;
      
      int access_flags = O_RDONLY;
      if (flags & Write_Access) access_flags = O_RDWR;
      access_flags |= O_CREAT | O_EXCL; // Create new file, fail if it exists

      int fd = open(path.value, access_flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
      if (fd == -1) return get_system_error();
      close(fd);
      return Ok();
    }
    case Resource_Type::Directory: {
      if (mkdir(path.value, 0755) == 0) return Ok();

      if (errno == EEXIST) return Ok();
      if (errno != ENOENT) return get_system_error();

      if (!flags.is_set(File_System_Flags::Force)) return get_system_error();

      const auto create_recursive = [] (auto&& self, char *path, usize length) -> Sys_Result<void> {
        struct stat st;
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) return Ok();

        char *separator = strrchr(path, '/');
        if (separator) {
          *separator = '\0';
          fin_check(self(self, path, separator - path));
          *separator = '/';
        }

        if (mkdir(path, 0755) != 0) {
          if (errno == EEXIST) return Ok();
          return get_system_error();
        }

        return Ok();
      };

      fin_ensure(path.length < PATH_MAX);
      char path_buffer[PATH_MAX];
      copy_memory(path_buffer, path.value, path.length);
      path_buffer[path.length] = '\0';

      return create_recursive(create_recursive, path_buffer, path.length);
    }
  }
}

static Sys_Result<bool> check_resource_exists (File_Path path, Option<Resource_Type> resource_type) {
  struct stat st;
  if (stat(path.value, &st) != 0) {
    if (errno == ENOENT) return false;
    return get_system_error();
  }

  if (resource_type.is_none()) return true;

  switch (resource_type.value) {
    case Resource_Type::File:      return Ok(S_ISREG(st.st_mode));
    case Resource_Type::Directory: return Ok(S_ISDIR(st.st_mode));
  }
}

static Sys_Result<void> delete_resource (File_Path path, Resource_Type resource_type) {
  switch (resource_type) {
    case Resource_Type::File: {
      if (unlink(path.value) != 0) {
        if (errno == ENOENT) return Ok();
        return get_system_error();
      }

      return Ok();
    }
    case Resource_Type::Directory: {
      if (rmdir(path.value) == 0) return Ok();

      if (errno == ENOENT) return Ok();
      if (errno == ENOTEMPTY || errno == EEXIST) {
        auto delete_recursive = [] (auto&& self, File_Path path) -> Sys_Result<void> {
          char buffer[2048];
          Memory_Arena arena { buffer };

          DIR *dir = opendir(path.value);
          if (!dir) return get_system_error();
          defer { closedir(dir); };

          struct dirent *entry;
          while ((entry = readdir(dir)) != NULL) {
            auto file_name = String(entry->d_name);
            if ((file_name != ".") && (file_name != "..")) {
              auto sub_path = make_file_path(arena, path, file_name);
              struct stat st;
              if (stat(sub_path.value, &st) != 0) return get_system_error();
              if (S_ISDIR(st.st_mode)) {
                fin_check(self(self, sub_path));
              } else {
                fin_check(delete_resource(sub_path, Resource_Type::File));
              }
            }
          }

          if (rmdir(path.value) != 0) return get_system_error();

          return Ok();
        };

        return delete_recursive(delete_recursive, path);
      }

      return get_system_error();
    }
  }
}

static Sys_Result<String> get_resource_name (File_Path path) {
  fin_ensure(path.length < PATH_MAX);

  int idx = path.length - 1;
  for (; idx >= 0; idx--) {
    if (path[idx] == '/') {
      auto after_separator = idx + 1;
      return String(path.value + after_separator, path.length - after_separator);
    }
  }

  return path;
}

static Sys_Result<File_Path> get_absolute_path (Memory_Arena &arena, File_Path path) {
  char resolved_path[PATH_MAX];
  if (!realpath(path.value, resolved_path)) {
    return get_system_error();
  }

  return copy_string(arena, String(resolved_path));
}

static bool is_absolute_path (File_Path path) {
  fin_ensure(!is_empty(path));

  return path[0] == '/';
}

static Sys_Result<Resource_Type> get_resource_type (File_Path path) {
  struct stat st;
  if (stat(path.value, &st) != 0) return get_system_error();

  if (S_ISDIR(st.st_mode)) return Resource_Type::Directory;
  if (S_ISREG(st.st_mode)) return Resource_Type::File;

  return Error(get_system_error());  // Handle other types if needed
}

static Sys_Result<File_Path> get_folder_path (Memory_Arena &arena, File_Path path) {
  fin_ensure(path.length < PATH_MAX);

  char buffer[PATH_MAX];
  copy_memory(buffer, path.value, path.length);
  buffer[path.length] = '\0';

  char *dir = dirname(buffer);

  if (!dir) return get_system_error();

  return copy_string(arena, String(dir));
}

static Sys_Result<File_Path> get_working_directory (Memory_Arena &arena) {
  char buffer[PATH_MAX];

  if (!getcwd(buffer, PATH_MAX)) return get_system_error();

  return copy_string(arena, String(buffer));
}

static Sys_Result<void> set_working_directory (File_Path path) {
  if (chdir(path.value) != 0) return get_system_error();
  return Ok();
}

static Sys_Result<void> for_each_file (File_Path directory, String extension, bool recursive, const Invocable<bool, File_Path> auto &func) {
  auto run_visitor = [extension, recursive, &func] (auto&& self, File_Path directory) -> Sys_Result<bool> {
    char buffer[2048];
    Memory_Arena arena { buffer };

    DIR *dir = opendir(directory.value);
    if (!dir) return get_system_error();
    defer { closedir(dir); };

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
      auto local = arena;

      auto file_name = String(entry->d_name);
      if (file_name == "." || file_name == "..") continue;

      auto sub_path = concat_string(local, directory, "/", file_name);

      struct stat st;
      if (stat(sub_path.value, &st) != 0) return get_system_error();

      if (S_ISDIR(st.st_mode)) {
        if (!recursive) continue;

        auto [error, should_continue] = self(self, sub_path);
        if (error)            return move(error.value);
        if (!should_continue) return false;
      }
      else {
        if (!ends_with(file_name, extension)) continue;
        if (!func(sub_path)) return false;
      }
    }

    return Ok(true);
  };

  fin_check(run_visitor(run_visitor, directory));

  return Ok();
}

static Sys_Result<List<File_Path>> list_files (Memory_Arena &arena, File_Path directory, String extension, bool recursive) {
  List<File_Path> file_list { arena };

  auto list_recursive = [&] (auto&& self, File_Path directory) -> Sys_Result<void> {
    DIR *dir = opendir(directory.value);
    if (!dir) return Error(get_system_error());
    defer { closedir(dir); };

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
      auto local = arena;

      auto file_name = String(entry->d_name);
      if (file_name == "." || file_name == "..") continue;

      auto sub_path = concat_string(local, directory, "/", file_name);

      struct stat st;
      if (stat(sub_path.value, &st) != 0) return get_system_error();

      if (S_ISDIR(st.st_mode)) {
        if (recursive) fin_check(self(self, sub_path));
      }
      else {
        if (!ends_with(file_name, extension)) continue;
              
        if (!file_list.contains(sub_path)) list_push(file_list, move(sub_path));
      }
    }

    return Ok();
  };

  fin_check(list_recursive(list_recursive, directory));

  return Ok(move(file_list));
}

static Sys_Result<void> copy_file (File_Path from, File_Path to) {
  char buffer[2048];
  Memory_Arena arena { buffer };

  File_Path folder_path;
  {
    auto [sys_error, path] = get_folder_path(arena, to);
    if (sys_error) return move(sys_error.value);

    folder_path = path;
  }

  {
    auto [sys_error, result] = check_resource_exists(folder_path, Resource_Type::Directory);
    if (sys_error) return move(sys_error.value);
    if (!result) create_resource(folder_path, Resource_Type::Directory, {});
  }

  int source_fd = open(from.value, O_RDONLY);
  if (source_fd == -1) return get_system_error();

  int dest_fd = open(to.value, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (dest_fd == -1) {
    close(source_fd);
    return get_system_error();
  }

  char buf[8192];
  ssize_t bytes_read, bytes_written;

  while ((bytes_read = read(source_fd, buf, sizeof(buf))) > 0) {
    char *out_ptr = buf;
    ssize_t remaining = bytes_read;

    while (remaining > 0) {
      bytes_written = write(dest_fd, out_ptr, remaining);
      if (bytes_written <= 0) {
        close(source_fd);
        close(dest_fd);
        return get_system_error();
      }
      remaining -= bytes_written;
      out_ptr += bytes_written;
    }
  }

  if (bytes_read == -1) {
    close(source_fd);
    close(dest_fd);
    return get_system_error();
  }

  close(source_fd);
  close(dest_fd);

  return Ok();
}

static Sys_Result<bool> is_file (File_Path path) {
  struct stat st;
  if (stat(path.value, &st) != 0) return get_system_error();
  return S_ISREG(st.st_mode);
}

static bool has_file_extension (File_Path path) {
  for (int i = path.length - 1; i >= 0; --i) {
    if (path.value[i] == '.') {
      if (i > 0 && i < path.length - 1) return true;
      break;
    }
  }

  return false;
}

static Sys_Result<bool> is_directory(File_Path path) {
  struct stat st;
  if (stat(path.value, &st) != 0) return get_system_error();
  return S_ISDIR(st.st_mode);
}

static Sys_Result<void> copy_directory (File_Path from, File_Path to) {
  auto copy_recursive = [] (auto&& self, File_Path from, File_Path to) -> Sys_Result<void> {
    char buffer[2048];
    Memory_Arena arena { buffer };

    DIR *dir = opendir(from.value);
    if (!dir) return get_system_error();
    defer { closedir(dir); };

    if (mkdir(to.value, 0755) != 0 && errno != EEXIST) return get_system_error();

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
      auto local = arena;

      auto file_name = String(entry->d_name);
      if (file_name == "." || file_name == "..") continue;

      auto from_path = make_file_path(local, from, file_name);
      auto to_path   = make_file_path(local, to,   file_name);

      struct stat st;
      if (stat(from_path.value, &st) != 0) return get_system_error();

      if (S_ISDIR(st.st_mode)) {
        fin_check(self(self, from_path, to_path));
      } else {
        fin_check(copy_file(from_path, to_path));
      }
    }

    return Ok();
  };

  fin_check(copy_recursive(copy_recursive, from, to));

  return Ok();
}

static Sys_Result<File> open_file (File_Path path, Bit_Mask<File_System_Flags> flags) {
  using enum File_System_Flags;

  int access_flags = O_RDONLY;
  if (flags & Write_Access) access_flags = O_RDWR;

  int creation_flags = 0;
  if (flags & Create_Missing) creation_flags |= O_CREAT;
  if (flags & Always_New)     creation_flags |= O_CREAT | O_TRUNC;

  int fd = open(path.value, access_flags | creation_flags, 0644);
  if (fd == -1) return get_system_error();

  return File { reinterpret_cast<void *>(fd), move(path) };
}

static Sys_Result<void> close_file (File &file) {
  if (close((int)(usize)(file.handle)) != 0) return get_system_error();
  file.handle = nullptr;
  return Ok();
}

static Sys_Result<u64> get_file_size (const File &file) {
  struct stat st;
  if (fstat((int)(usize)file.handle, &st) != 0) return get_system_error();

  return (u64) st.st_size;
}

static Sys_Result<u64> get_file_id (const File &file) {
  struct stat st;
  if (fstat((int)(usize)file.handle, &st) != 0) return get_system_error();

  return (u64) st.st_ino;
}

static Sys_Result<void> write_bytes_to_file (File &file, Byte_Type auto *bytes, usize count) {
  usize total_bytes_written = 0;
  while (total_bytes_written < count) {
    ssize_t bytes_written = write((int)(usize)file.handle, bytes + total_bytes_written, count - total_bytes_written);
    if (bytes_written <= 0) {
      return get_system_error();
    }
    total_bytes_written += bytes_written;
  }

  return Ok();
}

static Sys_Result<void> read_bytes_into_buffer (File &file, u8 *buffer, usize bytes_to_read) {
  fin_ensure(buffer);
  fin_ensure(bytes_to_read > 0);

  usize offset = 0;
  while (offset < bytes_to_read) {
    ssize_t bytes_read = read((int)(usize)file.handle, buffer + offset, bytes_to_read - offset);
    if (bytes_read <= 0) {
      if (bytes_read == 0) break;  // EOF
      return get_system_error();
    }
    offset += bytes_read;
  }

  return Ok();
}

static Sys_Result<Array<u8>> get_file_content (Memory_Arena &arena, File &file) {
  fin_check(reset_file_cursor(file));

  auto [sys_error, file_size] = get_file_size(file);
  if (sys_error)  return move(sys_error.value);
  if (!file_size) return Ok(Array<u8> {});

  auto buffer = reserve_array<u8>(arena, file_size, alignof(u8));

  usize offset = 0;
  while (offset < file_size) {
    ssize_t bytes_read = read((int)(usize)file.handle, buffer.values + offset, file_size - offset);
    if (bytes_read <= 0) {
      return get_system_error();
    }
    offset += bytes_read;
  }

  return buffer;
}

static Sys_Result<void> reset_file_cursor (File &file) {
  if (lseek((int)(usize)file.handle, 0, SEEK_SET) == (off_t)-1) return get_system_error();
  return Ok();
}

static Sys_Result<u64> get_last_update_timestamp (const File &file) {
  struct stat st;
  if (fstat((int)(usize)file.handle, &st) != 0) return get_system_error();

  return st.st_mtime;
}

static Sys_Result<File_Mapping> map_file_into_memory (const File &file) {
  auto [sys_error, mapping_size] = get_file_size(file);
  if (sys_error) return move(sys_error.value);
  if (mapping_size == 0) return File_Mapping {};

  void *memory = mmap(NULL, mapping_size, PROT_READ, MAP_PRIVATE, (int)(usize)file.handle, 0);
  if (memory == MAP_FAILED) return get_system_error();

  return File_Mapping {
    .handle = file.handle,
    .memory = reinterpret_cast<char *>(memory),
    .size   = mapping_size
  };
}

static Sys_Result<void> unmap_file (File_Mapping &mapping) {
  if (mapping.size == 0) return Ok();

  if (munmap(mapping.memory, mapping.size) != 0) return get_system_error();

  return Ok();
}

}
