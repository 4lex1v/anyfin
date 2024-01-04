
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/meta.hpp"
#include "anyfin/core/trap.hpp"

namespace Fin::Core {

struct None {};
constexpr inline None opt_none = None{};

template <typename T>
struct Option {
  using Value_Type = T;

  bool has_value = false;
  T    value;

  constexpr Option ()     = default;
  constexpr Option (None): has_value { false } {}

  constexpr Option (Value_Type &&_value)
    : has_value { true },
      value { move(_value) }
  {}

  constexpr Option (Option<T> &&other)
    : has_value { other.has_value },
      value     { move(other.value) }
  {
    destroy(other);
  }

  constexpr Option (const Option<T> &other)
    : has_value { true },
      value     { other.value }
  {}

  constexpr bool is_some () const { return this->has_value; }
  constexpr bool is_none () const { return !this->has_value; }

  constexpr operator bool () const { return is_some(); }

  constexpr Option<T>& operator = (Option<T> &&other) {
    destroy(*this);

    this->has_value = true;
    this->value     = move(other.value);

    destroy(other);

    return *this;
  }

  constexpr const Value_Type * operator -> () const {
    if (is_none()) return nullptr;
    return &this->value;
  }

  constexpr const Value_Type & get (const char *message = "Attempt to dereference an empty Option value") const {
    if (is_none()) [[unlikely]] trap(message);
    return this->value;
  }

  constexpr Value_Type & get (const char *message = "Attempt to dereference an empty Option value") {
    if (is_none()) [[unlikely]] trap(message);
    return this->value;
  }

  constexpr Value_Type && take (const char *message = "Attempt to deference an empty Option value") {
    if (is_none()) [[unlikely]] trap(message);
    return move(this->value); 
  }

  constexpr void handle_value (const Invocable<void, const T &> auto &closure) const {
    if (is_some()) closure(get());
  }
};

template <typename T>
static void destroy (Option<T> &option) {
  if (option) {
    if constexpr (Destructible<T>) destroy(option.value);
    else                           option.value.~T();

    option.has_value = false;
  }
}

}

