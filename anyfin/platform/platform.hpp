
#pragma once

#include "anyfin/base.hpp"

struct Memory_Region;
struct Status_Code;

Status_Code attach_console ();

/*
  NOTE: This call is NOT thread safe, there's no synchronization happening, thus calling it from different
  threads may cause bytes to be interleaved.
 */
Status_Code send_bytes_to_stdout (const Memory_Region &buffer);

Memory_Region reserve_virtual_memory (usize size);

void release_virtual_memory (Memory_Region &memory);

