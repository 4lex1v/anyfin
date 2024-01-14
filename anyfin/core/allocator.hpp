
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/callsite.hpp"
#include "anyfin/core/meta.hpp"

namespace Fin::Core {

struct Allocation_Request {
  enum Type { Reserve, Grow, Free };

  Type type;
  
  void *address = nullptr;
  usize old_size = 0;
  usize size;
  usize alignment = 0;
  bool  immediate = false;

  Callsite_Info callsite;

  Allocation_Request (const usize _size, const usize _alignment = alignof(void *), const Callsite_Info _info = {})
    : type { Reserve }, size { _size }, alignment { _alignment }, callsite { _info } {}

  Allocation_Request (void *_address, const usize _old_size, const usize _size,
                      bool _immediate = false, usize reserve_alignment = alignof(void*), const Callsite_Info _info = {})
    : type { Grow }, address { _address }, old_size { _old_size }, size { _size },
      immediate { _immediate }, alignment { reserve_alignment }, callsite { _info } {}

  Allocation_Request (void *_address, bool _immediate, const Callsite_Info _info = {})
    : type { Free }, address { _address }, immediate { _immediate }, callsite { _info } {}
};

template <typename A>
concept Allocator = requires (A &alloc, Allocation_Request request) {
  { allocator_dispatch(alloc, request) } -> Same_Types<u8 *>;
};

template <typename T = char>
fin_forceinline
static T * reserve (Allocator auto &allocator, const usize count = 1, const usize alignment = alignof(T), const Callsite_Info info = {}) {
  return reinterpret_cast<T*>(allocator_dispatch(allocator, Allocation_Request(sizeof(T) * count, alignment, info)));
}

/*
  
 */
template <typename T = char>
fin_forceinline
static T * grow (
  Allocator auto &allocator,
  T **address,
  usize old_size,
  usize new_size,
  /*
    'immediate' parameters enables some optimization opportunities when the underlying allocator is linear in nature
    and allocations happen sequentially in a loop. Effectively, it's a hint to the allocator that the caller makes
    several allocation calls sequentially.
   */
  bool immediate = false,
  usize alignment = alignof(void *),
  Callsite_Info info = {})
{
  auto memory =
    reinterpret_cast<T *>(
      allocator_dispatch(allocator, Allocation_Request(*address, old_size, new_size, immediate, alignment, info)));

  if (!*address && old_size == 0) [[unlikely]] *address = memory;
  return memory;
}

fin_forceinline
static void free (
  Allocator auto &allocator,
  void *address,
  /*
    
   */
  bool immediate = false,
  const Callsite_Info info = {})
{
  allocator_dispatch(allocator, Allocation_Request(address, immediate, info));
}

struct Allocator_View {
  using Dispatch_Func = u8 * (*) (void *, Allocation_Request);

  void *value            = nullptr;
  Dispatch_Func dispatch = nullptr;

  constexpr Allocator_View () = default;

  constexpr Allocator_View (const Allocator_View &other)
    : value { other.value }, dispatch { other.dispatch } {}
};

fin_forceinline
static u8 * allocator_dispatch (const Allocator_View &view, const Allocation_Request request) {
  return view.dispatch(view.value, move(request));
}

template <typename T>
concept Destructible = requires (T &value, Callsite_Info callsite) {
  { destroy(value, callsite) } -> Same_Types<void>;
};

template <typename T>
static void smart_destroy (T &value, Callsite_Info callsite = {}) {
  if constexpr (Destructible<T>) destroy(value, callsite); else value.~T();
}

struct Memory_Arena;

template <Allocator A>
static void alloc_destroy (auto &value, Callsite_Info callsite = {}) {
  if constexpr (!Same_Types<A, Memory_Arena>)
    smart_destroy(value, callsite);
}

}
