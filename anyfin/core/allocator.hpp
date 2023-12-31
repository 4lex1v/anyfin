
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/callsite.hpp"
#include "anyfin/core/memory.hpp"
#include "anyfin/core/meta.hpp"

namespace Fin::Core {

template <typename A>
concept Can_Reserve_Memory = requires (A allocator, usize size, usize alignment, const Callsite_Info callsite) {
  { reserve_memory(allocator, size, alignment, callsite) } -> Same_Types<u8 *>;
};

template <typename A>
concept Can_Free_Reservation = requires (A allocator, void *address, const Callsite_Info callsite) {
  { free_reservation(allocator, address, callsite) } -> Same_Types<void>;
};

template <typename A>
concept Can_Grow_Reseration = requires (A allocator, void *address, usize old_size, usize new_size, const Callsite_Info callsite) {
  { grow_reservation(allocator, address, old_size, new_size, callsite) } -> Same_Types<u8 *>;
};

template <typename A>
concept Allocator = Can_Reserve_Memory<A> && Can_Free_Reservation<A> && Can_Grow_Reseration<A>;

struct Allocator_View {
  using Reserve_Func = u8 * (*) (void *, usize, usize, Callsite_Info);
  using Grow_Func    = u8 * (*) (void *, void *, usize, usize, Callsite_Info);
  using Free_Func    = void (*) (void *, void *, Callsite_Info);

  void *underlying;
  
  Reserve_Func reserve;
  Grow_Func    grow;
  Free_Func    free;

  /*
    Needed to disable aggregate initialization, otherwise passing a templated allocator
    would cause a compilation error, in cases like:

      List (Allocator auto &_allocator)
        : allocator { _allocator } {}
   */
  constexpr Allocator_View () = default;
};

fin_forceinline
static u8 * reserve_memory (const Allocator_View &allocator, const usize size, const usize alignment = alignof(void *),
                            const Callsite_Info callsite = Callsite_Info()) {
  return allocator.reserve(allocator.underlying, size, alignment, callsite);
}

fin_forceinline
static u8 * grow_reservation (const Allocator_View &allocator, void * const address, const usize old_size, const usize new_size,
                                     const Callsite_Info callsite = Callsite_Info()) {
  return allocator.grow(allocator.underlying, address, old_size, new_size, callsite);
}

fin_forceinline
static void free_reservation (const Allocator_View &allocator, void * const address,
                                     const Callsite_Info callsite = Callsite_Info()) {
  allocator.free(allocator.underlying, address, callsite);
}

template <Allocator Alloc_Type>
struct Scope_Allocator {
  Scope_Allocator (Alloc_Type &underlying);
};

template <Allocator A> Scope_Allocator (Scope_Allocator<A>) -> Scope_Allocator<A>;

template <Allocator A>
static void destroy (Scope_Allocator<A> &allocator);

// template <typename T = char>
// constexpr T * reserve_array  (const Allocator &allocator, usize count, usize alignment = alignof(T)) {
//   return reinterpret_cast<T *>(reserve_memory(allocator, sizeof(T) * count, alignment));
// }

// template <typename T, typename... Args>
// static inline T* push_struct (const Allocator &allocator, Args&&... args) {
//   auto memory = reserve_memory(allocator, sizeof(T), alignof(T));
//   return new(memory) T { forward<Args>(args)... };
// }

}
