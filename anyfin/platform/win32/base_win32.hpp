
#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "anyfin/base.hpp"

#include "anyfin/core/meta.hpp"

#include "anyfin/platform/platform.hpp"

namespace Fin::Platform {

static System_Error get_system_error () {
  return System_Error { .error_code = GetLastError() };
}

}

