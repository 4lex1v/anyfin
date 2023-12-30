
#pragma once

#include "anyfin/core/meta.hpp"
#include "anyfin/core/trap.hpp"

namespace Fin::Core {

template <typename T>
struct Option {
  using Value_Type = T;

  const bool has_value = false;
  alignas(T) char storage[sizeof(T)] {};

  constexpr Option () = default;
  constexpr Option (Value_Type &&_value)
    : has_value { true }
  {
    new (this->storage) T (move(_value));
  }

  constexpr Option (const Option<T> &other)
    : has_value { true }
  {
    memcpy(this->storage, other.storage, sizeof(T));
  }

  constexpr bool is_some () const { return this->has_value; }
  constexpr bool is_none () const { return !this->has_value; }

  constexpr operator bool () const { return is_some(); }

  constexpr auto& operator = (Option<T> &&other) {
    if (this->has_value) {
      auto obj = reinterpret_cast<T *>(this->storage);
      obj->~T();
    }

    auto &obj = *reinterpret_cast<T *>(other.storage);
    new (this->storage) T { move(obj) };
    *const_cast<bool *>(&this->has_value) = true;

    *const_cast<bool *>(&other.has_value) = false;

    return *this;
  }

  constexpr const Value_Type * operator -> () const {
    if (is_none()) return nullptr;
    return reinterpret_cast<const Value_Type *>(this->storage);
  }

  constexpr const Value_Type & operator * () const & {
    if (is_none()) [[unlikely]] trap("Attempt to dereference an empty Option value");
    return get();
  }

  constexpr Value_Type & operator * () & {
    if (is_none()) [[unlikely]] trap("Attempt to dereference an empty Option value");
    return get();
  }

  constexpr Value_Type && operator * () && {
    if (is_none()) [[unlikely]] trap("Attempt to dereference an empty Option value");
    return move(reinterpret_cast<Value_Type *>(this->value));
  }

  constexpr Value_Type && take () {
    if (is_none()) [[unlikely]] trap("Attempt to deference an empty Option value");
    return move(*reinterpret_cast<Value_Type *>(this->storage));
  }

  constexpr const Value_Type & get (const char *message) const {
    if (is_none()) [[unlikely]] trap(message);
    return *reinterpret_cast<const Value_Type *>(this->storage);
  }

  constexpr const Value_Type & get () const {
    return get("Attempt to dereference an empty Option value");
  }

  constexpr Value_Type & get (const char *message) {
    if (is_none()) [[unlikely]] trap(message);
    return *reinterpret_cast<Value_Type *>(this->storage);
  }

  constexpr Value_Type & get () {
    return get("Attempt to dereference an empty Option value");
  }

  constexpr void handle_value (const Invocable<void, const T &> auto &closure) const {
    if (is_some()) closure(get());
  }
};

}
