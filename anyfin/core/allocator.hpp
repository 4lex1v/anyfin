
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/callsite.hpp"
#include "anyfin/core/meta.hpp"

namespace Fin::Core {

struct Memory_Arena;

struct Allocation_Request {
  enum Type { Reserve, Grow, Free };

  Type type;
  
  void *address = nullptr;
  usize old_size = 0;
  usize size;
  usize alignment = 0;

  Callsite_Info info;

  Allocation_Request (const usize _size, const usize _alignment = alignof(void *), const Callsite_Info _info = {})
    : type { Reserve }, size { _size }, alignment { _alignment }, info { _info } {}

  Allocation_Request (void *_address, const usize _old_size, const usize _size, const Callsite_Info _info = {})
    : type { Grow }, address { _address }, old_size { _old_size }, size { _size }, info { _info } {}

  Allocation_Request (void *_address, const Callsite_Info _info = {})
    : type { Free }, address { _address }, info { _info } {}
};

template <typename A>
concept Allocator = requires (A &alloc, Allocation_Request request) {
  { allocator_dispatch(alloc, request) } -> Same_Types<u8 *>;
};

template <typename T = char>
fin_forceinline
static T * reserve_memory (Allocator auto &allocator, const usize count = 1, const usize alignment = alignof(T), const Callsite_Info info = {}) {
  return reinterpret_cast<T *>(allocator_dispatch(allocator, Allocation_Request(sizeof(T) * count, alignment, info)));
}

fin_forceinline
static u8 * grow_reservation (Allocator auto &allocator, void *address, const usize old_size, const usize new_size, const Callsite_Info info = {}) {
  return allocator_dispatch(allocator, Allocation_Request(address, old_size, new_size, info));
}

fin_forceinline
static void free_reservation (Allocator auto &allocator, void *address, const Callsite_Info info = {}) {
  allocator_dispatch(allocator, Allocation_Request(address, info));
}

struct Allocator_View {
  using Dispatch_Func = u8 * (*) (void *, Allocation_Request);

  void *value            = nullptr;
  Dispatch_Func dispatch = nullptr;

  constexpr Allocator_View () = default;
};

fin_forceinline
static u8 * allocator_dispatch (const Allocator_View &view, const Allocation_Request request) {
  return view.dispatch(view.value, move(request));
}

template <Allocator A>
struct Scope_Allocator {
  Scope_Allocator (A &underlying);
};

template <Allocator A> Scope_Allocator (Scope_Allocator<A>) -> Scope_Allocator<A>;

template <Allocator A>
static void destroy (Scope_Allocator<A> &);

template <typename T>
concept Destructible = requires (T &value, Callsite_Info info) {
  { destroy(value, info) } -> Same_Types<void>;
};

template <typename T>
static void smart_destroy (T &value, Callsite_Info info = Callsite_Info()) {
  if constexpr (Destructible<T>) destroy(value, info); else value.~T();
}

}
