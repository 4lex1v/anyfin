
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/allocator.hpp"
#include "anyfin/core/option.hpp"
#include "anyfin/core/result.hpp"

namespace Fin::Core {

template <typename T>
struct List {
  using Value_Type = T;
  
  struct Node {
    Value_Type  value;
    Node       *next;
    Node       *previous;
  };

  Allocator_View allocator;

  Node  *first = nullptr;
  Node  *last  = nullptr;

  usize count = 0;

  List () = default;

  List (Allocator auto &_allocator)
    : allocator { _allocator } {}

  struct Iterator {
    Node *node;

    Iterator (Node *_node): node { _node } {}

    bool operator != (const Iterator &other) const {
      return this->node != other.node;
    }

    Iterator& operator ++ () {
      node = node->next;
      return *this;
    }

    Value_Type&       operator * ()       { return node->value; }
    const Value_Type& operator * () const { return node->value; }
  };

  Iterator begin () { return Iterator(first); } 
  Iterator end   () { return Iterator(nullptr); }

  const Iterator begin () const { return Iterator(first); } 
  const Iterator end   () const { return Iterator(nullptr); }

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

    return {};
  }

  bool contains (const Invocable<bool, const T &> auto &pred) const {
    return !!find_node(pred);
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
static inline void destroy (List<T> &list, const Callsite_Info info = Callsite_Info()) {
  auto cursor = list.first;
  while (cursor) {
    destroy(cursor->value, info);
    auto next = cursor->next;
    free_reservation(list.allocator, cursor);
    cursor = next;
  }
}

template <typename T>
using List_Value = typename List<T>::Value_Type;

template <typename T>
static void list_push (List<T> &list, List_Value<T> &&value) {
  using Node_Type = typename List<T>::Node;

  auto reservation = reserve_memory(list.allocator, sizeof(Node_Type), alignof(Node_Type));
  auto node = new (reservation) Node_Type { .value = move(value) };

  if (list.first == nullptr) {
    list.first = node;
    list.last  = node;
  }
  else {
    node->previous  = list.last;
    list.last->next = node;
    list.last       = node;
  }

  list.count += 1;
}

template <typename T>
static inline void list_push_copy (List<T> &list, const List_Value<T> &value) {
  List_Value<T> copied = value;
  list_push(list, move(copied));
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

} // namespace Fin::Core
