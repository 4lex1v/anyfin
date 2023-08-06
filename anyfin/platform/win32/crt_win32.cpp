
#include "anyfin/base.hpp"

#pragma function(memset)
extern "C" void * memset (void *destination, int value, size_t count) {
  auto storage = reinterpret_cast<u8 *>(destination);
  for (size_t idx = 0; idx < count; idx++) {
    storage[idx] = static_cast<u8>(value);
  }

  return destination;
}

#pragma function(memcpy)
extern "C" void * memcpy (void *destination, const void *source, size_t count) {
  auto from = reinterpret_cast<const u8 *>(source);
  auto to   = reinterpret_cast<u8 *>(destination);
  for (size_t idx = 0; idx < count; idx++) {
    to[idx] = from[idx];
  }

  return destination;
}
