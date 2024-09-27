
#define FIN_PROCESS_HPP_IMPL

#include "anyfin/process.hpp"

namespace Fin {

[[noreturn]] static void terminate (u32 exit_code) {
  _exit(exit_code);
  __builtin_unreachable();
}

};
