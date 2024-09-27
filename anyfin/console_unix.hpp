#define FIN_CONSOLE_HPP_IMPL

#include "anyfin/console.hpp"
// No need to include "anyfin/win32.hpp"

#include <unistd.h>
#include <errno.h>
#include <cstring>

namespace Fin {

static Sys_Result<void> write_to_stdout (String message) {
  ssize_t total_bytes_written = 0;

  while (total_bytes_written < message.length) {
    ssize_t bytes_written = write(STDOUT_FILENO, message.value + total_bytes_written, message.length - total_bytes_written);
    if (bytes_written == -1) {
      return get_system_error();
    }
    total_bytes_written += bytes_written;
  }

  /*
    Flushing stdout buffer if necessary. Since we're using unbuffered write(), this is generally not needed.
    If using buffered I/O (e.g., printf), you would use fflush(stdout).
  */

#ifdef DEV_BUILD
  // For debug builds, write the message to stderr or use syslog
  write(STDERR_FILENO, message.value, message.length);
#endif

  return Ok();
}

}
