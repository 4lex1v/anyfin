
#define FIN_STARTUP_HPP_IMPL

#include "anyfin/strings.hpp"
#include "anyfin/option.hpp"
#include "anyfin/startup.hpp"

namespace Fin {

struct Command_Line_Input {
  String program_name;
  Option<String> arguments_string;
};

static Command_Line_Input get_command_line () {
  auto input = GetCommandLineA();

  auto cursor = input;
  for (; *cursor && *cursor != ' '; cursor++);

  auto program_name_length = cursor - input;

  /*
    I've noticed that on Windows there might be space characters after the program's name before the terminating 0...
   */
  while (*cursor == ' ') cursor += 1;

  // No arguments were passed.
  if (!*cursor) return Command_Line_Input { String(input, program_name_length) };

  auto args_string = cursor;
  usize args_string_length = 0;
  while (cursor[args_string_length]) args_string_length += 1;

  return Command_Line_Input {
    .program_name     = String(input, program_name_length),
    .arguments_string = Option(String(args_string, args_string_length))
  };
}

static void collect_input_arguments(const String &command_line, Array<Startup_Argument> &args) {
  const auto is_whitespace = [] (char value) { return value == ' ' || value == '\t'; };
  const char *cursor = command_line.value;

  for (usize arg_index = 0; arg_index < args.count && cursor && *cursor; ++arg_index) {
    while (*cursor && is_whitespace(*cursor)) ++cursor;

    const char *start = cursor;
    while (*cursor && !is_whitespace(*cursor)) ++cursor;

    String token(start, cursor - start);
    s32 eq_offset = -1;
    for (s32 i = 0; i < s32(token.length); i++) {
      if (token[i] == '=') { eq_offset = i; break; }
    }

    if (eq_offset > 0) {
      args[arg_index] = Startup_Argument {
        .type  = Startup_Argument::Type::Pair,
        .key   = String(token.value, eq_offset),
        .value = String(token.value + eq_offset + 1, token.length - (eq_offset + 1)),
      };
    } else {
      args[arg_index] = Startup_Argument {
        .type = Startup_Argument::Type::Value,
        .key  = token
      };
    }
  }
}

static u32 count_arguments (const String &input) {
  if (is_empty(input)) return 0;

  auto cursor = input.value;

  int count = 0, in_token = 0;
  for (; cursor && *cursor; ++cursor) {
    const auto is_space_or_tab = *cursor == ' ' || *cursor == '\t';
    count += (!in_token && !is_space_or_tab);
    in_token = !is_space_or_tab;
  }

  return count;
}

static Array<Startup_Argument> get_startup_args (Memory_Arena &arena) {
  auto [has_arguments, arguments] = get_command_line().arguments_string;
  if (!has_arguments) return {};

  auto args_count = count_arguments(arguments);
  if (!args_count) return {};

  auto args = reserve_array<Startup_Argument>(arena, args_count);
  collect_input_arguments(arguments, args);

  return args;
}


}
