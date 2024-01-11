
#pragma once

#include "anyfin/base.hpp"

#include "anyfin/platform/platform.hpp"

namespace Fin::Platform {

struct System_Command_Status {
  Core::String output;
  u32          status_code;
};

static Result<System_Command_Status> run_system_command (Core::Allocator auto &allocator, Core::String_View command_line);

}

#ifndef COMMANDS_HPP_IMPL
  #ifdef PLATFORM_WIN32
    #include "commands_win32.hpp"
  #else
    #error "Unsupported platform"
  #endif
#endif
