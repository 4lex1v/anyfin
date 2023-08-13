
#include <cstring>

#include "anyfin/core/arena.hpp"
#include "anyfin/core/result.hpp"
#include "anyfin/core/strings.hpp"

#include "anyfin/platform/files.hpp"
#include "anyfin/platform/win32/common_win32.hpp"

Result<File_Path> make_file_path_from_array (Memory_Arena &arena, const String *segments, usize count) {
  usize reservation_size = 0;
  for (usize idx = 0; idx < count; idx++) {
    auto length = segments[idx].length;
    if (length == 0) continue;

    reservation_size += length + 1;
  }

  auto buffer = reserve_array<char>(arena, reservation_size);
  if (buffer == nullptr) return Status_Code { Status_Code::Runtime_Error };

  auto cursor = buffer;
  for (usize idx = 0; idx < count; idx++) {
    const auto &segment = segments[idx];

    memcpy(cursor, segment.value, segment.length);
    cursor[segment.length] = '\\';

    cursor += segment.length + 1;
  }

  buffer[reservation_size - 1] = '\0';

  return File_Path { buffer, reservation_size - 1 };
}

Result<File> open_file (const File_Path &path, const Bit_Mask<Open_File_Flags> flags) {
  use(Status_Code);
  using enum Open_File_Flags;

  auto access  = GENERIC_READ    | ((flags & Write_Access) ? GENERIC_WRITE    : 0);
  auto sharing = FILE_SHARE_READ | ((flags & Shared_Write) ? FILE_SHARE_WRITE : 0); 
  auto status  = (flags & Create_Missing) ? OPEN_ALWAYS : OPEN_EXISTING;
  
  auto handle = CreateFile(path.value, access, sharing, NULL, status, FILE_ATTRIBUTE_NORMAL, NULL);
  if (handle == INVALID_HANDLE_VALUE) {
    if (status == OPEN_EXISTING && GetLastError() == ERROR_FILE_NOT_FOUND)
      return Status_Code { Status_Code::Runtime_Error };
    return get_system_error();
  }

  return File { reinterpret_cast<File::Handle *>(handle), path };
}
