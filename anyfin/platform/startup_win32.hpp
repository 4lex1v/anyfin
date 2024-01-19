
#define STARTUP_HPP_IMPL

#include "anyfin/core/strings.hpp"
#include "anyfin/core/option.hpp"

#include "anyfin/platform/startup.hpp"

namespace Fin::Platform {

struct Command_Line_Input {
  Core::String_View program_name;
  Core::Option<Core::String_View> arguments_string;
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
  if (!*cursor) return Command_Line_Input { Core::String_View(input, program_name_length) };

  auto args_string = cursor;
  usize args_string_length = 0;
  while (cursor[args_string_length]) args_string_length += 1;

  return Command_Line_Input {
    .program_name     = Core::String_View(input, program_name_length),
    .arguments_string = Core::Option(Core::String_View(args_string, args_string_length))
  };
}

static void collect_input_arguments(const Core::String_View &command_line, Core::Array<Startup_Argument> &args) {
  const auto is_whitespace = [] (char value) { return value == ' ' || value == '\t'; };
  const char *cursor = command_line.value;

  for (usize arg_index = 0; arg_index < args.count && cursor && *cursor; ++arg_index) {
    while (*cursor && is_whitespace(*cursor)) ++cursor;

    const char *start = cursor;
    while (*cursor && !is_whitespace(*cursor)) ++cursor;

    Core::String_View token(start, cursor - start);
    s32 eq_offset = -1;
    for (s32 i = 0; i < s32(token.length); i++) {
      if (token[i] == '=') { eq_offset = i; break; }
    }

    if (eq_offset > 0) {
      args[arg_index] = Startup_Argument {
        .type  = Startup_Argument::Type::Pair,
        .key   = Core::String_View(token.value, eq_offset),
        .value = Core::String_View(token.value + eq_offset + 1, token.length - (eq_offset + 1)),
      };
    } else {
      args[arg_index] = Startup_Argument {
        .type = Startup_Argument::Type::Value,
        .key  = token
      };
    }
  }
}

static u32 count_arguments (const Core::String_View &input) {
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

static Core::Array<Startup_Argument> get_startup_args (Core::Allocator auto &allocator) {
  auto [has_arguments, arguments] = get_command_line().arguments_string;
  if (!has_arguments) return {};

  auto args_count = count_arguments(arguments);
  if (!args_count) return {};

  auto args = reserve_array<Startup_Argument>(allocator, args_count);
  collect_input_arguments(arguments, args);

  return args;
}


}
