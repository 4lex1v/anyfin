
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
  
  Reserve_Func _reserve;
  Grow_Func    _grow;
  Free_Func    _free;

  constexpr Allocator_View () = default;
};

[[clang::always_inline]]
static inline u8 * reserve_memory (const Allocator_View &allocator, const usize size, const usize alignment = alignof(void *),
                                   const Callsite_Info callsite = Callsite_Info()) {
  return allocator._reserve(allocator.underlying, size, alignment, callsite);
}

[[clang::always_inline]]
static inline u8 * grow_reservation (const Allocator_View &allocator, void * const address, const usize old_size, const usize new_size,
                                     const Callsite_Info callsite = Callsite_Info()) {
  return allocator._grow(allocator.underlying, address, old_size, new_size, callsite);
}

[[clang::always_inline]]
static inline void free_reservation (const Allocator_View &allocator, void * const address,
                                     const Callsite_Info callsite = Callsite_Info()) {
  allocator._free(allocator.underlying, address, callsite);
}



// template <typename T = char>
// constexpr T * reserve_array  (const Allocator &allocator, usize count, usize alignment = alignof(T)) {
//   return reinterpret_cast<T *>(reserve_memory(allocator, sizeof(T) * count, alignment));
// }

// template <typename T, typename... Args>
// static inline T* push_struct (const Allocator &allocator, Args&&... args) {
//   auto memory = reserve_memory(allocator, sizeof(T), alignof(T));
//   return new(memory) T { forward<Args>(args)... };
// }

template <typename T>
struct Scope_Allocator {
  Scope_Allocator (T underlying);
  ~Scope_Allocator ();

  // u8 * reserve (usize size, usize alignment, const Callsite_Info callsite = Callsite_Info());
  // void free    (void *address, const Callsite_Info callsite = Callsite_Info());
  // u8 * grow    (void *address, usize old_size, usize new_size, const Callsite_Info callsite = Callsite_Info());
};

template <typename T>
static u8 * scope_allocator_reserve (Scope_Allocator<T> &allocator, const usize size, const usize alignment,
                                     const Callsite_Info callsite = Callsite_Info()) {
  return allocator.alloc(size, alignment, callsite);
}

template <typename T>
static void scope_allocator_free (Scope_Allocator<T> &allocator, void * const address,
                                  const Callsite_Info callsite = Callsite_Info()) {
  allocator.free(address, callsite);
}

template <typename T>
static u8 * scope_allocator_grow (Scope_Allocator<T> &allocator, void * const address, const usize old_size, const usize new_size,
                                  const Callsite_Info callsite = Callsite_Info()) {
  return allocator.grow(address, old_size, new_size, callsite);
}

// template <typename T>
// static Allocator scoped_allocator (Scope_Allocator<T> &allocator) {
//   using ReserveFuncPtr = u8 * (*) (Scope_Allocator<T>&, usize, usize, const Callsite_Info);
//   using FreeFuncPtr    = void (*) (Scope_Allocator<T>&, void *, const Callsite_Info);
//   using GrowFuncPtr    = u8 * (*) (Scope_Allocator<T>&, void *, usize, usize, const Callsite_Info);

//   ReserveFuncPtr reserve = scope_allocator_reserve<T>;
//   FreeFuncPtr    free    = scope_allocator_free<T>;
//   GrowFuncPtr    grow    = scope_allocator_grow<T>;

//   return Allocator {
//     .underlying = &allocator,
//     .alloc      = reinterpret_cast<Allocator::Reserve_Func>(reserve),
//     .free       = reinterpret_cast<Allocator::Free_Func>(free),
//     .grow       = reinterpret_cast<Allocator::Grow_Func>(grow),
//   };
// }

}
