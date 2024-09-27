#define FIN_PLATFORM_HPP_IMPL

#include "anyfin/strings.hpp"
#include "anyfin/platform.hpp"
// No need to include "anyfin/win32.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <errno.h>
#include <cstring>

namespace Fin {

static Platform get_platform_type () { return Platform::Unix; }

static u32 get_system_error_code () {
  return errno;
}

static System_Error get_system_error (Convertible_To<const char *> auto&&... args) {
  auto error_code = get_system_error_code();

  char buffer[256];
#if defined(__APPLE__) || (_POSIX_C_SOURCE >= 200112L) && ! _GNU_SOURCE
  strerror_r(error_code, buffer, sizeof(buffer));
  const char *message = buffer;
#else
  const char *message = strerror_r(error_code, buffer, sizeof(buffer));
#endif

  return System_Error { String(message), error_code };
}

static void destroy (System_Error error) {
  // No action needed since we didn't allocate memory
}

static u32 get_logical_cpu_count () {
  long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
  if (nprocs < 1) {
    // Fallback method using sysctl on BSD systems
#ifdef __APPLE__
    int mib[2] = { CTL_HW, HW_NCPU };
    int numCPU = 0;
    size_t len = sizeof(numCPU);
    sysctl(mib, 2, &numCPU, &len, NULL, 0);
    if (numCPU < 1) numCPU = 1;
    return numCPU;
#else
    return 1;
#endif
  }
  return nprocs;
}

extern char **environ;

static Sys_Result<Option<String>> get_env_var (Memory_Arena &arena, String name) {
  // Create a C-string copy of the name
  char *name_cstr = reserve<char>(arena, name.length + 1);
  copy_memory(name_cstr, name.value, name.length);
  name_cstr[name.length] = '\0';

  usize name_length = name.length;

  // Iterate over the environment variables
  for (char **env = environ; *env != nullptr; ++env) {
    char *env_entry = *env;

    // Check if the current entry starts with the variable name followed by '='
    if (strncmp(env_entry, name_cstr, name_length) == 0 && env_entry[name_length] == '=') {
      // Environment variable found, now extract the value part
      char *value = env_entry + name_length + 1;
      usize value_length = strlen(value);

      // Allocate memory for the value
      char *value_copy = reserve<char>(arena, value_length);
      copy_memory(value_copy, value, value_length);

      return Ok(Option(String(value_copy, value_length)));
    }
  }

  // Environment variable not found, return an empty Option
  return Ok(Option(String()));
}

}
