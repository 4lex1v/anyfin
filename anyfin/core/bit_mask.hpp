
#pragma once

#include "anyfin/base.hpp"

namespace Fin::Core {

template <typename E, typename Mask = u64>
struct Bit_Mask {
  Mask bit_mask;

  consteval Bit_Mask (): bit_mask { 0 } {}
  consteval Bit_Mask (Mask value): bit_mask { value } {}
  consteval Bit_Mask (E value): bit_mask { static_cast<Mask>(value) } {}

  consteval Bit_Mask<E> operator | (E value) { return bit_mask | static_cast<Mask>(value); }

  consteval bool operator & (E value) const { return bit_mask & static_cast<Mask>(value); }

  consteval void set    (E value)       { this->bit_mask |= static_cast<Mask>(value); }
  consteval bool is_set (E value) const { return this->bit_mask & static_cast<Mask>(value); }
};

template <typename E>
constinit static inline Bit_Mask<E> Empty_Mask = static_cast<Bit_Mask<E>>(0);

template <typename E>
consteval Bit_Mask<E> operator | (E left, E right) { return Bit_Mask(left) | right; }

}
