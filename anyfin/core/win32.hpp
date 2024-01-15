
#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>  

#include "anyfin/base.hpp"

namespace Fin::Core::Win32_Internals {

/*
  TODO: Thread UNSAFE implementation
 */
static u32 stdout_print (const char *message, usize length) {
  auto stdout = GetStdHandle(STD_OUTPUT_HANDLE);

  DWORD bytes_written = 0;
  while (true) {
    auto bytes_to_write = length - bytes_written;

    auto cursor = message + bytes_written;
    if (!WriteFile(stdout, cursor, bytes_to_write, &bytes_written, nullptr)) {
      return GetLastError();
    }

    if (bytes_written == length) break;
  }

  /*
    Not checking the error for this one, cause according to the docs it would return an error
    for attempting to flush a console output, which is what this function may be used for most
    often. This may be helpful when we pipe std out, though.
   */
  FlushFileBuffers(stdout);

#ifdef DEV_BUILD
  OutputDebugString(message);
#endif

  return 0;
}

}

