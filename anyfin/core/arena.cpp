
#include "anyfin/core/arena.hpp"

namespace Fin::Core {

u8 * Memory_Arena::reserve (const usize size, const usize alignment, const Callsite_Info info) {
  assert(is_power_of_2(alignment));
  assert(size > 0);
  assert(alignment > 0);
  
  if (size == 0)      [[unlikely]] return nullptr;
  if (alignment == 0) [[unlikely]] return nullptr;
  
  auto base         = this->memory + this->offset;
  auto aligned_base = align_forward(base, alignment);

  auto alignment_shift  = static_cast<usize>(aligned_base - base);
  auto reservation_size = alignment_shift + size;
  
  if ((reservation_size + this->offset) > this->size) [[unlikely]] return nullptr;

  this->offset += reservation_size;

  return aligned_base;
}

u8 * Memory_Arena::grow (void *address, const usize old_size, const usize new_size, const Callsite_Info info) {
  const auto estimate_alignment = [] (const void* ptr) -> usize {
    if (!ptr) return 1;
    return 1ULL << __builtin_ctzll(reinterpret_cast<usize>(ptr));
  };

  const auto alignment = estimate_alignment(address);

  auto new_region = this->reserve(new_size, alignment, info);
  if (!new_region) return nullptr;

  memcpy_s(new_region, new_size, address, old_size);

  return new_region;
}

[[clang::always_inline]]
static u8 * arena_allocator_reserve (Memory_Arena &arena, const usize size, const usize alignment, const Callsite_Info info) {
  return arena.reserve(size, alignment, info);
}

[[clang::always_inline]]
static void arena_allocator_free (Memory_Arena &arena, void *address, const Callsite_Info info) {
  arena.free(address, info);
}

[[clang::always_inline]]
static u8 * arena_allocator_grow (Memory_Arena &arena, void *address, const usize old_size, const usize new_size, const Callsite_Info info) {
  return arena.grow(address, old_size, new_size, info);
}

Memory_Arena::operator Allocator () {
  return Allocator {
    .underlying = this,
    ._reserve   = reinterpret_cast<Allocator::Reserve_Func>(arena_allocator_reserve),
    ._grow      = reinterpret_cast<Allocator::Grow_Func>(arena_allocator_grow),
    ._free      = reinterpret_cast<Allocator::Free_Func>(arena_allocator_free),
  };
}

}
