
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

inline void * operator new (usize, void *value) { return value; }

#define defer auto tokenpaste(__deferred_lambda_call, __COUNTER__) = Fin::Base::deferrer << [&] ()

namespace Fin::Base {

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
