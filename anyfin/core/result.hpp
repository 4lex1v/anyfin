
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/strings.hpp"
//#include "anyfin/core/closure.hpp"
#include "anyfin/core/trap.hpp"

namespace Fin::Core {

struct String_View;

template <typename T>
struct Error {
  using Value_Type = T;

  Value_Type value;

  constexpr Error (const Value_Type &_value): value { _value } {}
  constexpr Error (Value_Type &&_value): value { move(_value) } {}

  template <typename... Args>
  constexpr Error (Args&&... args)
    : value { Value_Type(forward<Args>(args)...) }
  {}
};

template <typename T>
struct Ok {
  using Value_Type = T;

  Value_Type value;

  constexpr Ok (const Value_Type &_value): value { _value } {}
  constexpr Ok (Value_Type &&_value): value { move(_value) } {}

  template <typename... Args>
  constexpr Ok (Args&&... args)
    : value { Value_Type(forward<Args>(args)...) }
  {}
};

template <>
struct Ok<void> {
  using Value_Type = void;
  constexpr Ok () {};
};

/*
  Deduction guide for the void specialization.
 */
Ok() -> Ok<void>;

struct Tag {
  enum struct Value { Error, Success, Empty };
  using enum Value;

  Value value;

  operator bool () const { return value == Success; }

  Tag (Value _value): value { _value } {}
};

template <typename S, typename T>
struct Result {
  using Status_Type = S;
  using Value_Type  = T;

  template <typename U>
  using rebind = Result<S, U>;

  Tag         tag;
  Status_Type status;
  Value_Type  value;

  constexpr Result (Error<S>&& error): tag { Tag::Error }, status { move(error.value) } {}
  constexpr Result (Status_Type &&status): tag { Tag::Error }, status { move(status) } {}

  constexpr Result (Ok<T>&& ok): tag { Tag::Success }, value { move(ok.value) } {}
  constexpr Result (Value_Type &&value): tag { Tag::Success }, value { move(value) } {}

  constexpr ~Result () {
    switch (tag.value) {
      case Tag::Empty: { break; }
      case Tag::Error: {
        this->status.~Status_Type();
        break;
      }
      case Tag::Success: {
        this->value.~Value_Type();
        break;
      }
    }
  }

  constexpr operator bool () const { return tag.value == Tag::Success; }

  Value_Type& get (const char *trap_message) {
    if (this->tag.value != Tag::Success) [[unlikely]] trap(trap_message);
    return this->value;
  }

  Value_Type& get () { return get("Attempt to access value on a failed result container"); }

  Value_Type&& take (const char *trap_message) {
    if (this->tag.value != Tag::Success) [[unlikely]] trap(trap_message);
    this->tag = Tag::Empty;
    return move(this->value);
  }

  Value_Type&& take () {
    return this->take("Attempt to move a value from a failed result container");
  }

  Value_Type&& operator * () && {
    return move(*this).take("Attempt to move a value from a failed result container");
  }

  constexpr void handle_value (const Invocable<void, const Value_Type &> auto &func) const {
    if (this->tag.value == Tag::Success) [[likely]] func(this->value);
  }

  template <typename To>
  constexpr Result<Status_Type, To> as () && {
    assert(this->tag.value != Tag::Empty);

    if (this->tag.value == Tag::Error)
      return rebind<To>(Error(move(this->status)));
    else
      return rebind<To>(Ok(move(*reinterpret_cast<To *>(&this->value))));
  }
};

template <typename S>
struct Result<S, void> {
  using Status_Type = S;
  using Value_Type  = void;

  Tag         tag;
  Status_Type status;

  constexpr Result (Error<S>&& error): tag { Tag::Error }, status { move(error.value) } {}
  constexpr Result (Status_Type &&status): tag { Tag::Error }, status { move(status) } {}

  constexpr Result (Ok<void>&& ok): tag { Tag::Success } {}

  constexpr operator bool () const { return tag.value == Tag::Success; }

  void expect (const String_View &message) { if (tag.value == Tag::Error) trap(message.value); }
  void expect () { expect("Failed result value"); }
};

#define check(RESULT) do { if (auto result = (RESULT); !result) return result; } while (0)

}

