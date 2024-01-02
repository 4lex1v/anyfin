
#pragma once

#include "anyfin/base.hpp"

constexpr bool is_power_of_2 (const auto &value) {
  return (value > 0) && ((value & (value - 1)) == 0);
}
