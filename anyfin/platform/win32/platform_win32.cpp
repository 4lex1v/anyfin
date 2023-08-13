
#include "anyfin/core/prelude.hpp"
#include "anyfin/core/status_code.hpp"

#include "anyfin/platform/platform.hpp"
#include "anyfin/platform/win32/common_win32.hpp"

Status_Code attach_console () {
  if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
    if (!AllocConsole()) return get_system_error();

    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT);
  }
  return Status_Code::Success;
}

Status_Code send_bytes_to_stdout (const Memory_Region &buffer) {
  auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
  if (handle == INVALID_HANDLE_VALUE) return get_system_error();

  DWORD bytes_written = 0;
  if (!WriteFile(handle, buffer.memory, buffer.size, &bytes_written, nullptr))
    return get_system_error();

  return Status_Code::Success;
}

Memory_Region reserve_virtual_memory (usize size) {
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);

  auto aligned_size = align_forward(size, system_info.dwPageSize);

  auto memory = VirtualAlloc(0, aligned_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

  return Memory_Region { (u8 *) memory, aligned_size };
}

void release_virtual_memory (Memory_Region &memory) {
  VirtualFree(memory.memory, memory.size, MEM_RELEASE);
}
