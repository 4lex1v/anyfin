
#pragma once

#include "anyfin/base.hpp"

namespace Fin::Core {

template <typename E, typename Mask = u64>
struct Bit_Mask {
  Mask bit_mask;

  constexpr Bit_Mask (): bit_mask { 0 } {}
  constexpr Bit_Mask (Mask value): bit_mask { value } {}
  constexpr Bit_Mask (E value): bit_mask { static_cast<Mask>(value) } {}
  
  constexpr Bit_Mask<E> operator | (E value) const { return bit_mask | static_cast<Mask>(value); }
 
  constexpr bool operator & (E value) const { return bit_mask & static_cast<Mask>(value); }

  constexpr void set    (E value)       { this->bit_mask |= static_cast<Mask>(value); }
  constexpr bool is_set (E value) const { return this->bit_mask & static_cast<Mask>(value); }
};

template <typename E>
constexpr Bit_Mask<E> operator | (E left, E right) { return Bit_Mask(left) | right; }

}
