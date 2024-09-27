#define FIN_ATOMICS_HPP_IMPL

#include "anyfin/atomics.hpp"

namespace Fin {

template <typename T>
using Atomic_Value = typename Atomic<T>::Value_Type;

#define fin_compiler_barrier() asm volatile("" ::: "memory")

// Atomic Load
template <Memory_Order order = Memory_Order::Relaxed, typename T>
static T atomic_load(const Atomic<T>& atomic) {
  static_assert(sizeof(T) <= sizeof(void*));
  static_assert(order == Memory_Order::Relaxed || order == Memory_Order::Acquire || order == Memory_Order::Sequential);

  T result;
  if constexpr (order == Memory_Order::Relaxed) {
    asm volatile("ldr %0, %1"
                 : "=r"(result)
                 : "m"(atomic.value)
                 :);
  }
  else if constexpr (order == Memory_Order::Acquire) {
    asm volatile("ldar %0, %1"
                 : "=r"(result)
                 : "Q"(atomic.value)
                 : "memory");
  }
  else if constexpr (order == Memory_Order::Sequential) {
    asm volatile("dmb ish" ::: "memory");
    asm volatile("ldar %0, %1"
                 : "=r"(result)
                 : "Q"(atomic.value)
                 : "memory");
    asm volatile("dmb ish" ::: "memory");
  }

  return result;
}

// Atomic Store
template <Memory_Order order = Memory_Order::Relaxed, typename T>
static void atomic_store(Atomic<T>& atomic, Atomic_Value<T> value) {
  static_assert(sizeof(T) <= sizeof(void*));
  static_assert(order == Memory_Order::Relaxed || order == Memory_Order::Release || order == Memory_Order::Sequential);

  if constexpr (order == Memory_Order::Relaxed) {
    asm volatile("str %w0, %1"
                 :
                 : "r"(value), "m"(atomic.value)
                 :);
  }
  else if constexpr (order == Memory_Order::Release) {
    asm volatile("stlr %w0, %1"
                 :
                 : "r"(value), "Q"(atomic.value)
                 : "memory");
  }
  else if constexpr (order == Memory_Order::Sequential) {
    asm volatile("dmb ish" ::: "memory");
    asm volatile("stlr %w0, %1"
                 :
                 : "r"(value), "Q"(atomic.value)
                 : "memory");
    asm volatile("dmb ish" ::: "memory");
  }
}

// Atomic Fetch Add
template <Memory_Order order = Memory_Order::Relaxed, typename T>
static T atomic_fetch_add(Atomic<T>& atomic, T value) {
  static_assert(sizeof(T) <= sizeof(void*));

  T old_value, new_value;
  unsigned int status;
  if constexpr (order == Memory_Order::Relaxed) {
    do {
      asm volatile(
        "ldxr %w0, %2\n"
        "add  %w1, %w0, %w4\n"
        "stxr %w3, %w1, %2"
        : "=&r"(old_value),    // %0
          "=&r"(new_value),    // %1
          "+Q"(atomic.value),  // %2
          "=&r"(status)        // %3
        : "r"(value)           // %4
        : "memory");
    } while (status != 0);
  } else {
    do {
      asm volatile(
        "ldaxr %w0, %2\n"
        "add   %w1, %w0, %w4\n"
        "stlxr %w3, %w1, %2"
        : "=&r"(old_value),    // %0
          "=&r"(new_value),    // %1
          "+Q"(atomic.value),  // %2
          "=&r"(status)        // %3
        : "r"(value)           // %4
        : "memory");
    } while (status != 0);
  }
  return old_value;
}

template <Memory_Order order = Memory_Order::Relaxed, typename T>
static T atomic_fetch_sub(Atomic<T>& atomic, T value) {
  return atomic_fetch_add<order>(atomic, -value);
}

template <Memory_Order success = Memory_Order::Acquire_Release,
          Memory_Order failure = Memory_Order::Acquire,
          typename T>
static bool atomic_compare_and_set(Atomic<T>& atomic, Atomic_Value<T> expected, Atomic_Value<T> new_value) {
  static_assert(sizeof(T) <= sizeof(void*));

  unsigned int status;
  T tmp;
  if constexpr (success == Memory_Order::Relaxed && failure == Memory_Order::Relaxed) {
    asm volatile(
      "ldxr %w1, %2\n"
      "cmp %w1, %w3\n"
      "b.ne 1f\n"
      "stxr %w0, %w4, %2\n"
      "1:"
      : "=&r"(status), "=&r"(tmp), "+Q"(atomic.value)
      : "r"(expected), "r"(new_value)
      : "cc", "memory");
  }
  else {
    asm volatile(
      "ldaxr %w1, %2\n"
      "cmp %w1, %w3\n"
      "b.ne 1f\n"
      "stlxr %w0, %w4, %2\n"
      "1:"
      : "=&r"(status), "=&r"(tmp), "+Q"(atomic.value)
      : "r"(expected), "r"(new_value)
      : "cc", "memory");
  }

  return (status == 0);
}

} // namespace Fin
