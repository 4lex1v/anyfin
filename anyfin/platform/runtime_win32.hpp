
#include "anyfin/base.hpp"

#include "anyfin/core/arena.hpp"
#include "anyfin/core/lifecycle.hpp"
#include "anyfin/core/option.hpp"
#include "anyfin/core/strings.hpp"
#include "anyfin/core/trap.hpp"

/*
  Defined in Win32 kernel32 library.
 */
extern "C" const char * GetCommandLineA();

namespace Fin::Core {
static inline int launch();
}

extern "C" int mainCRTStartup () {
  return Fin::Core::launch();
}

namespace Fin::Core {

struct Command_Line_Input {
  String_View         program_name;
  Option<String_View> arguments_string;
};

static inline Command_Line_Input get_command_line () {
  auto input = GetCommandLineA();

  auto cursor = input;
  for (; *cursor && *cursor != ' '; cursor++);

  auto program_name_length = cursor - input;

  /*
    I've noticed that on Windows there might be space characters after the program's name before the terminating 0...
   */
  while (*cursor == ' ') cursor += 1;

  // No arguments were passed.
  if (!*cursor) return Command_Line_Input { String_View(input, program_name_length) };

  auto args_string = cursor;
  usize args_string_length = 0;
  while (cursor[args_string_length]) args_string_length += 1;

  return Command_Line_Input {
    .program_name     = String_View(input, program_name_length),
    .arguments_string = Option(String_View(args_string, args_string_length))
  };
}

static inline usize find_value_end_position (const String_View &input) {
  usize offset = 0;
  while (input[offset] != ' ') offset += 1;

  return offset;
}

static inline void collect_input_arguments (const String_View &command_line, Startup_Argument args[], usize args_count) {
  const auto skip_whitespace = [] (const String_View &input) -> usize {
    usize offset = 0;
    while (input[offset] == ' ') offset += 1;
    return offset;
  };

  const auto parse_argument = [] (const String_View &arg) -> auto {
    s32 eq_offset = -1;
    for (s32 i = 0; i < s32(arg.length); i++) {
      if (arg[i] == '=') { eq_offset = i; break; }
    }

    if (eq_offset == 0) assert(false); // TODO: Handle bad argument

    if (eq_offset > 0) return Startup_Argument {
      .type  = Startup_Argument::Type::Pair,
      .key   = String_View(arg.value, eq_offset - 1),
      .value = String_View(),
    };

    return Startup_Argument {
      .type = Startup_Argument::Type::Value,
      .key  = arg
    };
  };

  usize count = 0;

  String_View cursor = command_line;
  for (usize arg_index = 0; arg_index < args_count; arg_index++) {
    auto value_start = cursor + skip_whitespace(cursor);
    if (!value_start[0]) break;
    
    auto value_length = find_value_end_position(value_start);

    args[arg_index] = parse_argument(String_View(value_start.value, value_length));
  }
}

static inline u32 count_arguments (const String_View &input) {
  u32 arguments_count = 0;

  auto cursor = input.value;
  while (true) {
    while (*cursor && *cursor != ' ') cursor += 1;
    if (!*cursor) break; // end of input reached

    arguments_count += 1;
    while (*cursor && *cursor == ' ') cursor += 1;
  }

  return arguments_count;
}

static inline int launch () {
  assert(false); // Setup default global allocator
  
  auto [_, arguments_string_opt] = get_command_line();

  if (!arguments_string_opt) return app_entry({});

  auto arguments_string = *arguments_string_opt;
  auto count = count_arguments(arguments_string);

  Startup_Argument args[count];
  collect_input_arguments(arguments_string, args, count);

  return app_entry(Slice(args, count));
}

}

