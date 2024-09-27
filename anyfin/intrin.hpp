
#pragma once

#if defined(CPU_ARCH_X64) and defined(PLATFORM_WIN32)
  #include <intrin.h>
#elif defined(CPU_ARCH_X64) and defined(PLATFORM_UNIX)
  #include <x86intrin.h>
#elif defined(CPU_ARCH_ARM64)
  #include <arm_neon.h>
#endif
