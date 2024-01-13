
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/core/strings.hpp"
#include "anyfin/core/trap.hpp"

namespace Fin::Core {

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

  constexpr operator bool (this auto self) { return self.value == Error; }

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

  constexpr operator bool () const { return tag.value == Tag::Success; }

  constexpr bool is_error (this auto &self) { return self.tag.value == Tag::Error; }
  constexpr bool is_ok    (this auto &self) { return self.tag.value == Tag::Success; }

  Value_Type & get (const Invocable<void, const Status_Type &> auto &func) {
    if (this->tag.value != Tag::Success) [[unlikely]] func(this->status);
    return this->value;
  }

  Value_Type & get (String_View trap_message = "Attempt to access value of a failed result\n") {
    return get([trap_message] (auto &) { trap(trap_message); });
  }

  Value_Type && take (const Invocable<String_View, const Status_Type &> auto &func) {
    if (this->tag.value != Tag::Success) [[unlikely]] trap(func(this->status));
    this->tag = Tag::Empty;
    return move(this->value);
  }

  Value_Type && take (String_View trap_message = "Attempt to access value of a failed result\n") {
    if (this->tag.value != Tag::Success) [[unlikely]] trap(trap_message);
    this->tag = Tag::Empty;
    return move(this->value);
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

  void expect (String_View message = "Failed result value") {
    if (this->tag.value != Tag::Success) [[unlikely]] trap(message);
  }

  void expect (const Invocable<String_View, const Status_Type &> auto &func) {
    expect(func(this->status));
  }
};

template <typename S>
struct Result<S, bool> {
  using Status_Type = S;
  using Value_Type  = void;

  Tag         tag;
  Status_Type status;
  bool        value;

  constexpr Result (Error<S>&& error): tag { Tag::Error }, status { move(error.value) } {}
  constexpr Result (Status_Type &&status): tag { Tag::Error }, status { move(status) } {}

  constexpr Result (Ok<bool>&& ok): tag { Tag::Success }, value { ok.value } {}
  constexpr Result (bool _value): tag { Tag::Success }, value { _value } {}

  constexpr operator bool () const { return tag.value == Tag::Success; }

  bool check (String_View message = "Failed result value") {
    if (this->tag.value != Tag::Success) [[unlikely]] trap(message);
    return value;
  }

  bool check (const Invocable<String_View, const Status_Type &> auto &func) {
    return check(func(this->status));
  }
};

template <typename S, typename T>
static void destroy (Result<S, T> &result, Callsite_Info callsite = {}) {
  switch (result.tag.value) {
    case Tag::Empty:   { break; }
    case Tag::Error:   { smart_destroy(result.status, callsite); break; }
    case Tag::Success: { smart_destroy(result.value); break; }
  }
}

/*
  
 */
#define fin_check(RESULT)                                           \
  do {                                                              \
    if (auto result = (RESULT); !result)                            \
      return ::Fin::Core::Error(::Fin::Core::move(result.status));  \
  } while (0)

}

