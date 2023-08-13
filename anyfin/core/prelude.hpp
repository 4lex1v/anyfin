
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/assert.hpp"
#include "anyfin/core/meta.hpp"

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

template <typename T>
constexpr bool is_power_of_2 (const T &value) {
  return (value > 0) && ((value & (value - 1)) == 0);
}

template <typename T, usize N>
consteval usize array_count_elements (const T (&)[N]) {
  return N;
}

template <typename T>
constexpr T align_forward (const T value, const usize by) {
  assert(is_power_of_2(by));

  if constexpr (!is_pointer<T>)
    return (value + (by - 1)) & ~(by - 1);

  return reinterpret_cast<T>((reinterpret_cast<usize>(value) + (by - 1)) & ~(by - 1));
}

struct Memory_Region {
  u8    *memory;
  usize  size;
};

template <typename Type>
class Deferrable {
  Type cleanup;

public:
  explicit Deferrable (Type &&cb): cleanup{ forward<Type>(cb) } {}
  ~Deferrable () { cleanup(); }
};

static struct {
  template <typename Type>
  Deferrable<Type> operator << (Type &&cb) {
    return Deferrable<Type>(forward<Type>(cb));
  }
} deferrer;

#define defer auto tokenpaste(__deferred_lambda_call, __COUNTER__) = deferrer << [&]

template <typename E>
struct Bit_Mask {
  using Mask = u64;
  
  Mask bit_mask;

  constexpr Bit_Mask (): bit_mask { 0 } {}
  constexpr Bit_Mask (Mask value): bit_mask { value } {}
  constexpr Bit_Mask (E value): bit_mask { static_cast<Mask>(value) } {}

  constexpr Bit_Mask<E> operator | (E value) { return bit_mask | static_cast<Mask>(value); }

  constexpr bool operator & (E value) const { return bit_mask & static_cast<Mask>(value); }

  constexpr void set    (E value)       { this->bit_mask |= static_cast<Mask>(value); }
  constexpr bool is_set (E value) const { return this->bit_mask & static_cast<Mask>(value); }
};
