
#pragma once

namespace Fin::Core {

namespace internals {

template <typename T> struct Remove_Ref       { using type = T; };
template <typename T> struct Remove_Ref<T &>  { using type = T; };
template <typename T> struct Remove_Ref<T &&> { using type = T; };

template <typename T> struct Remove_Ptr      { using type = T; };
template <typename T> struct Remove_Ptr<T *> { using type = T; };

template <typename T> struct Raw_Type               { using type = T; };
template <typename T> struct Raw_Type<T &&>         { using type = T; };
template <typename T> struct Raw_Type<T &>          { using type = T; };
template <typename T> struct Raw_Type<T *>          { using type = T; };
template <typename T> struct Raw_Type<T * const>    { using type = T; };
template <typename T> struct Raw_Type<T * volatile> { using type = T; };

}

template <typename A, typename B> inline constexpr bool same_types       = false;
template <typename A>             inline constexpr bool same_types<A, A> = true;

template <typename T> inline constexpr bool is_pointer = false;
template <typename T> inline constexpr bool is_pointer<T *> = true;
template <typename T> inline constexpr bool is_pointer<T * const> = true;
template <typename T> inline constexpr bool is_pointer<T * volatile> = true;
template <typename T> inline constexpr bool is_pointer<T * const volatile> = true;

template <typename From, typename To>
inline constexpr bool is_convertible = __is_convertible_to(From, To);

template <typename T> constexpr T&& materialize () {}

template <typename T> using remove_ref = typename internals::Remove_Ref<T>::type;
template <typename T> using remove_ptr = typename internals::Remove_Ptr<T>::type;
template <typename T> using raw_type   = typename internals::Raw_Type<T>::type;

template <typename T>
constexpr remove_ref<T> && move (T &&value) {
  return static_cast<remove_ref<T> &&>(value);
}

template <typename T> constexpr T&& forward (remove_ref<T>  &value) { return static_cast<T &&>(value); }
template <typename T> constexpr T&& forward (remove_ref<T> &&value) { return static_cast<T &&>(value); }

template <typename A, typename B>
concept Same_Types = same_types<A, B>;

template <typename A, typename B>
concept Convertible_To = is_convertible<A, B>;

template <typename F, typename Ret, typename... Args>
concept Invocable = requires (F func, Args... args) {
  { func(args...) } -> Same_Types<Ret>;
};

}
