
#pragma once

#include "anyfin/base.hpp"

template <typename T> struct Remove_Ref      { using Type = T; };
template <typename T> struct Remove_Ref<T&>  { using Type = T; };
template <typename T> struct Remove_Ref<T&&> { using Type = T; };

template <typename T> using remove_ref = typename Remove_Ref<T>::Type;

template <typename T> struct Remove_Ptr      { using Type = T; };
template <typename T> struct Remove_Ptr<T *> { using Type = T; };

template <typename T> using remove_ptr = typename Remove_Ptr<T>::Type;

template <typename T> struct Is_Pointer     { static constexpr bool value = false; };
template <typename T> struct Is_Pointer<T*> { static constexpr bool value = true;  };

template <typename T> constexpr inline bool is_pointer = Is_Pointer<T>::value;

template <typename A, typename B> struct Same_Type       { static constexpr bool value = false; };
template <typename T>             struct Same_Type<T, T> { static constexpr bool value = true;  };

template <typename A, typename B> constexpr inline bool same_types = Same_Type<A, B>::value;

template <typename T> fin_forceinline constexpr T&& forward (remove_ref<T> &value)  { return static_cast<T&&>(value); }
template <typename T> fin_forceinline constexpr T&& forward (remove_ref<T> &&value) { return static_cast<T&&>(value); }

template <typename T> fin_forceinline constexpr T && move (T &ref) { return static_cast<T &&>(ref); }
