
#pragma once

#include "anyfin/base.hpp"

namespace Fin::Core {

struct String_View;

template <typename T, typename... Args>
constexpr const T & min (const T &first, const Args &... args) {
  const T &smallest = (first < ... < args);
  return smallest;
}

template <typename T, typename... Args>
constexpr const T & max (const T &first, const Args &... args) {
  const T &biggest = (first > ... > args);
  return biggest;
}

template <typename N>
consteval N kilobytes (const N value) {
  return value * 1024;
}

template <typename N>
consteval N megabytes (const N value) {
  return kilobytes(value) * 1024;
}

template <typename T, usize N>
consteval usize array_count_elements (const T (&)[N]) {
  return N;
}

}
