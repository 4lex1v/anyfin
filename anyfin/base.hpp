
#pragma once

#include <cstdint>

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

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
