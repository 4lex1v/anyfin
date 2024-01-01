
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/callsite.hpp"
#include "anyfin/core/memory.hpp"
#include "anyfin/core/meta.hpp"

namespace Fin::Core {

struct Heap;
struct Memory_Arena;

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

struct Allocation_Request {
  enum Type { Reserve, Grow, Free };
  
  void *address = nullptr;
  usize old_size = 0;
  usize new_size;
  usize alignment = 0;
  Callsite_Info info;

  constexpr Allocation_Request (const usize _size, const usize _alignment = alignof(void *), const Callsite_Info _info = {})
    : new_size { _size }, alignment { _alignment }, info { _info } {}

  constexpr Allocation_Request (void *_address, const usize _old_size, const usize _new_size, const Callsite_Info _info = {})
    : address { _address }, old_size { _old_size }, new_size { _new_size }, info { _info } {}

  constexpr Allocation_Request (void *_address, const Callsite_Info _info = {})
    : address { _address }, info { _info } {}
};

template <typename A>
concept Allocator = requires (A &alloc, Allocation_Request request) {
  { allocator_dispatch(alloc, request) } -> Same_Types<u8 *>;
};

struct Allocator_View;

fin_forceinline
static u8 * reserve_memory (Allocator auto &allocator, const usize size, const usize alignment, const Callsite_Info info) {
  return allocator_dispatch<Allocation_Request::Reserve>(allocator, Allocation_Request(size, alignment, info));
}

fin_forceinline
static u8 * grow_reservation (Allocator auto &allocator, void *address, const usize old_size, const usize new_size, const Callsite_Info info) {
  return allocator_dispatch<Allocation_Request::Grow>(allocator, Allocation_Request(address, old_size, new_size, info));
}

fin_forceinline
static void free_reservation (Allocator auto &allocator, void *address, const Callsite_Info info) {
  allocator_dispatch<Allocation_Request::Free>(allocator, Allocation_Request(address, info));
}

struct Allocator_View {
  using Dispatch_Func = u8 * (*) (void *, Allocation_Request);

  void *value;
  Dispatch_Func dispatch;
};

fin_forceinline
static u8 * allocator_dispatch (const Allocator_View &view, const Allocation_Request request) {
  return view.dispatch(view.value, move(request));
}

// template <Allocator Alloc_Type>
// struct Scope_Allocator {
//   Scope_Allocator (Alloc_Type &underlying);
// };

// template <Allocator A> Scope_Allocator (Scope_Allocator<A>) -> Scope_Allocator<A>;

// template <Allocator A>
// static void destroy (Scope_Allocator<A> &allocator);

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
