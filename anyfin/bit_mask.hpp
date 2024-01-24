
#pragma once

#include "anyfin/base.hpp"

namespace Fin {

template <typename E, typename Mask = u64>
struct Bit_Mask {
  Mask bit_mask;

  constexpr Bit_Mask (): bit_mask { static_cast<Mask>(0) } {}
  constexpr Bit_Mask (Mask value): bit_mask { value } {}
  constexpr Bit_Mask (E value): bit_mask { static_cast<Mask>(value) } {}
  
  constexpr Bit_Mask<E> operator | (this auto self, E value) { return self.bit_mask | static_cast<Mask>(value); }
  constexpr bool        operator & (this auto self, E value) { return self.bit_mask & static_cast<Mask>(value); }
 
  constexpr Bit_Mask<E> set    (this auto self, E value) { return self |= value; }
  constexpr bool        is_set (this auto self, E value) { return self & value; }
};

template <typename E>
constexpr Bit_Mask<E> operator | (E left, E right) { return Bit_Mask(left) | right; }

}
