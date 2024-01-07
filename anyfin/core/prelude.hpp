
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/meta.hpp"

namespace Fin::Core {

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

constexpr decltype(auto) min (const auto &first, const auto &&... args) { return (first < ... < args); }
constexpr decltype(auto) max (const auto &first, const auto &&... args) { return (first > ... > args); }

consteval auto kilobytes (Integral auto value) { return value * 1024; }
consteval auto megabytes (Integral auto value) { return kilobytes(value) * 1024; }

template <typename T, usize N>
consteval usize array_count_elements (const T (&)[N]) {
  return N;
}

constexpr bool is_power_of_2 (Integral auto value) {
  return (value > 0) && ((value & (value - 1)) == 0);
}

}
