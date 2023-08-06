
#include "cbuild.h"

extern "C" bool setup_project (const Arguments *args, Project *project) {
  add_global_compiler_options(project, "-std=c++20", "-O0 -g -gcodeview -DDEBUG_BUILD");

  add_global_include_search_path(project, ".");

  set_toolchain(project, Toolchain_Type_LLVM);

  auto window = add_static_library(project, "window");
  add_source_file(window, "anyfin/window/win32/window_win32.cpp");

  auto platform = add_static_library(project, "platform");
  add_all_sources_from_directory(platform, "anyfin/platform/win32", "cpp", false);

  auto launcher = add_static_library(project, "launcher");
  add_source_file(launcher, "anyfin/launcher/win32/main_win32.cpp");

  auto fury = add_executable(project, "fury");
  add_source_file(fury, "demos/squares_of_fury/game.cpp");
  add_linker_option(fury, "/debug:full /subsystem:Windows");
  link_with(fury, platform, window, launcher, "kernel32.lib user32.lib");

  return true;
}
