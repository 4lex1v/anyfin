
#pragma once

#include "anyfin/base.hpp"

namespace Fin::Core {

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

template <typename E>
constexpr static inline Bit_Mask<E> Empty_Mask = static_cast<Bit_Mask<E>>(0);

template <typename E>
constexpr Bit_Mask<E> operator | (E left, E right) { return Bit_Mask(left) | right; }

}
