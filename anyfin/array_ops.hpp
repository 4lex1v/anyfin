
#pragma once

#include "anyfin/base.hpp"
#include "anyfin/array.hpp"
#include "anyfin/option.hpp"
#include "anyfin/intrin.hpp"

namespace Fin {

#if defined(CPU_ARCH_X64)

static Option<usize> find_offset (Array<u64> data, const u64 _value) {
  if (is_empty(data)) return opt_none;
  
  fin_ensure(is_aligned_by(data.values, 32));

  auto value = _mm256_set1_epi64x(_value);

  s32 step  = 4;
  s32 limit = data.count - step;

  s32 idx = 0;
  while (idx <= limit) {
    auto array  = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(data.values + idx));
    auto result = _mm256_cmpeq_epi64(array, value);

    auto match = _mm256_movemask_epi8(result);
    if (match) return ((__builtin_ctz(match) >> 3) + idx);

    idx += step;
  }

  for (usize i = idx; i < data.count; i++)
    if (data[i] == _value) return i;

  return opt_none;
}

#elif defined(CPU_ARCH_ARM64)

static Option<usize> find_offset(Array<u64> data, const u64 _value) {
  if (is_empty(data)) return opt_none;
  
  // Ensure the data is 16-byte aligned for NEON instructions
  fin_ensure(is_aligned_by(data.values, 16));

  // Set up the value vector with two copies of _value
  uint64x2_t value = vdupq_n_u64(_value);

  s32 step = 4;                    // Process 4 elements per iteration
  s32 limit = data.count - step;

  s32 idx = 0;
  while (idx <= limit) {
    // Load 4 u64 values into two NEON vectors (128 bits each)
    uint64x2x2_t array = vld1q_u64_x2(data.values + idx);

    // Compare the loaded vectors with the value vector
    uint64x2_t result0 = vceqq_u64(array.val[0], value);
    uint64x2_t result1 = vceqq_u64(array.val[1], value);

    // Check if any elements in result0 match
    uint64_t res0 = vgetq_lane_u64(result0, 0);
    uint64_t res1 = vgetq_lane_u64(result0, 1);

    if (res0 != 0) return idx;
    if (res1 != 0) return idx + 1;

    // Check if any elements in result1 match
    uint64_t res2 = vgetq_lane_u64(result1, 0);
    uint64_t res3 = vgetq_lane_u64(result1, 1);

    if (res2 != 0) return idx + 2;
    if (res3 != 0) return idx + 3;

    idx += step;
  }

  // Process any remaining elements individually
  for (usize i = idx; i < data.count; i++) {
    if (data[i] == _value) return i;
  }

  return opt_none;
}
#endif


static bool contains_key (const Array<u64> &data, u64 key) {
  return find_offset(data, key).is_some();
}

}

