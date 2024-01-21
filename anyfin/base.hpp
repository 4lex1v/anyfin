
#pragma once

using s8  = signed char;
using s16 = short;
using s32 = int;
using s64 = long long;

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using b1 = u8;
using b2 = u16;
using b4 = u32;
using b8 = u64;

using f32 = float;
using f64 = double;

using usize = size_t;
using psize = usize;

#define tokenpaste2(X, Y) X##Y
#define tokenpaste(X, Y) tokenpaste2(X, Y)

#define stringify2(X) #X
#define stringify(X) stringify2(X)

#define flag(N) 1 << (N)

#define fin_forceinline __attribute__((always_inline))

#if DEV_BUILD
  #define fin_deb_skip fin_forceinline
#else
  #define fin_deb_skip
#endif

#define defer auto tokenpaste(__deferred_lambda_call, __COUNTER__) = Fin::Base::deferrer << [&] ()

#define fn(NAME) [&] (auto NAME)
#define lambda fn(it)
#define block  fn(_)

namespace Fin::Base {

template <typename T, usize N>
consteval usize array_count_elements (const T (&)[N]) {
  return N;
}

template <typename Type>
struct Deferrable {
  Type cleanup;

  explicit Deferrable (Type &&cb): cleanup { cb } {}
  ~Deferrable () { cleanup(); }
};

static struct {
  template <typename Type>
  constexpr Deferrable<Type> operator << (Type &&cb) {
    return Deferrable<Type>(static_cast<Type &&>(cb));
  }
} deferrer;

}
