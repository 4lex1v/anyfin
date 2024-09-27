#define FIN_COMMANDS_HPP_IMPL

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "anyfin/commands.hpp"
#include "anyfin/defer.hpp"

namespace Fin {

static Sys_Result<System_Command_Status> run_system_command (Memory_Arena &arena, String command_line) {
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    return get_system_error();
  }

  pid_t pid = fork();
  if (pid == -1) {
    close(pipefd[0]);
    close(pipefd[1]);
    return get_system_error();
  }

  if (pid == 0) {
    // Child process
    // Close the read end of the pipe
    close(pipefd[0]);

    // Redirect stdout and stderr to the write end of the pipe
    if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
      _exit(errno);
    }
    if (dup2(pipefd[1], STDERR_FILENO) == -1) {
      _exit(errno);
    }

    // Close the write end of the pipe, as it's duplicated now
    close(pipefd[1]);

    // Prepare the command and arguments
    char *cmdline_copy = reserve<char>(arena, command_line.length + 1);
    memcpy(cmdline_copy, command_line.value, command_line.length);
    cmdline_copy[command_line.length] = '\0';

    // Simple argument parsing (space-separated)
    char *argv[128];  // Maximum of 128 arguments
    int argc = 0;

    char *token = strtok(cmdline_copy, " ");
    while (token != NULL && argc < 127) {
      argv[argc++] = token;
      token = strtok(NULL, " ");
    }
    argv[argc] = NULL;  // Null-terminate the array

    // Execute the command
    execvp(argv[0], argv);

    // If execvp returns, an error occurred
    _exit(errno);
  } else {
    // Parent process
    // Close the write end of the pipe
    close(pipefd[1]);

    auto output_buffer = get_memory_at_current_offset<char>(arena);
    usize output_size = 0;

    char read_buffer[4096];
    ssize_t bytes_read;

    while ((bytes_read = read(pipefd[0], read_buffer, sizeof(read_buffer))) > 0) {
      char *region = reserve<char>(arena, bytes_read);
      fin_ensure(region);
      
      memcpy(region, read_buffer, bytes_read);
      output_size += bytes_read;
    }

    if (bytes_read == -1) {
      close(pipefd[0]);
      return get_system_error();
    }

    close(pipefd[0]);

    // Wait for the child process to finish
    int status;
    if (waitpid(pid, &status, 0) == -1) {
      return get_system_error();
    }

    int exit_code = 0;
    if (WIFEXITED(status)) {
      exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
      exit_code = 128 + WTERMSIG(status);
    }

    // Null-terminate the output buffer
    if (output_size > 0) {
      // Remove trailing CRLF if present
      if (output_size >= 2 && output_buffer[output_size - 2] == '\r' && output_buffer[output_size - 1] == '\n') {
        output_size -= 2;
      }
      output_buffer[output_size] = '\0';
    }

    return Ok(System_Command_Status {
      .output      = String(output_buffer, output_size),
      .status_code = exit_code,
    });
  }
}

}
