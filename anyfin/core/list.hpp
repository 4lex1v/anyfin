
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/assert.hpp"
#include "anyfin/core/allocator.hpp"
#include "anyfin/core/option.hpp"

namespace Fin::Core {

template <typename T>
struct List {
  using Value_Type = T;
  
  struct Node {
    Value_Type  value;
    Node       *next;
    Node       *previous;

    void * operator new (usize size, Allocator auto &allocator) {
      return reserve<Node>(allocator);
    }
  };

  Allocator_View allocator;

  Node  *first = nullptr;
  Node  *last  = nullptr;

  usize count = 0;

  constexpr List () = default;

  constexpr List (Allocator auto &_allocator)
    : allocator { _allocator } {}

  struct Iterator {
    Node *node;

    constexpr Iterator (Node *_node): node { _node } {}

    constexpr bool operator != (const Iterator &other) const {
      return this->node != other.node;
    }

    constexpr Iterator& operator ++ () {
      node = node->next;
      return *this;
    }

    constexpr Value_Type&       operator * ()       { return node->value; }
    constexpr const Value_Type& operator * () const { return node->value; }
  };

  constexpr Iterator begin () { return Iterator(first); } 
  constexpr Iterator end   () { return Iterator(nullptr); }

  constexpr const Iterator begin () const { return Iterator(first); } 
  constexpr const Iterator end   () const { return Iterator(nullptr); }

  fin_forceinline
  constexpr void for_each (const Invocable<void, T &> auto &func) const {
    for (auto node = first; node; node = node->next) func(node->value);
  }

  Option<Node *> find_node (const Invocable<bool, const T &> auto &pred) const {
    auto node = this->first;
    while (node) {
      if (pred(node->value)) return node;
      node = node->next;
    }

    return {};
  }

  Option<const T &> find (const Invocable<bool, const T &> auto &pred) const {
    auto node = find_node(pred);
    if (node) return Option(node.value);
    return opt_none;
  }

  bool contains (const Invocable<bool, const T &> auto &pred) const {
    return !!find_node(pred);
  }
  
  bool contains (const T &value) const
    requires requires (const T &a, const T &b) { { a == b } -> Same_Types<bool>; }
  {
    return contains([&] (const T &elem) { return elem == value; });
  }

  bool remove (const Invocable<bool, const T &> auto &pred) {
    auto [found, node] = find_node(pred);
    if (!found) return false;

    if (node->previous != nullptr) node->previous->next = node->next;
    else                           this->first = node->next;

    if (node->next != nullptr) node->next->previous = node->previous;
    else                       this->last = node->previous;

    this->count--;

    return true;
  }

};

template <typename T>
constexpr void destroy (List<T> &list, const Callsite_Info info = Callsite_Info()) {
  auto cursor = list.first;
  while (cursor) {
    smart_destroy(cursor->value, info);
    auto next = cursor->next;
    free(list.allocator, cursor);
    cursor = next;
  }
}

template <typename T>
using List_Value = typename List<T>::Value_Type;

template <typename T>
static T & list_push (List<T> &list, List_Value<T> &&value) {
  using Node_Type = typename List<T>::Node;

  assert(list.allocator.value != nullptr);

  auto node = new (list.allocator) Node_Type { .value = move(value) };

  if (list.first == nullptr) {
    assert(list.last == nullptr);
    list.first = node;
    list.last  = node;
  }
  else {
    node->previous  = list.last;
    list.last->next = node;
    list.last       = node;
  }

  list.count += 1;

  return node->value;
}

template <typename T>
static T & list_push_copy (List<T> &list, List_Value<T> value) {
  return list_push(list, move(value));
}

// template <typename T, typename P>
// static Pair<bool, usize> find_position (const List<T> *list, P predicate) {
//   auto node = list->first;
//   for (usize idx = 0; idx < list->count; ++idx) {
//     if (predicate(&(node->value))) return { true, idx };
//     node = node->next;
//   }
//   return { false, 0 };
// }

template <typename T>
static bool list_remove_at_index (List<T> &list, usize position) {
  if (list->count == 0 || position >= list->count) return false;

  auto node = list->first;
  for (usize i = 0; i < position; ++i) node = node->next;

  if (node->previous != nullptr) node->previous->next = node->next;
  else                           list->first = node->next;

  if (node->next != nullptr) node->next->previous = node->previous;
  else                       list->last = node->previous;

  list->count--;

  return true;
}

template <typename T>
constexpr bool is_empty (const List<T> &list) {
  return list.first == nullptr;
}

namespace iterator {

template <typename T>
constexpr usize count (const List<T> &list) {
  return list.count;
}

}

} // namespace Fin::Core
