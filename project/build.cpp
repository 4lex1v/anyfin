
#include "cbuild.h"

extern "C" bool setup_project (const Arguments *args, Project *project) {
  add_global_compiler_option(project, "/nologo");
  add_global_archiver_option(project, "/nologo");
  add_global_linker_option(project, "/nologo");

  add_global_include_search_path(project, ".");

  auto launcher = add_static_library(project, "launcher");
  add_source_file(launcher, "launcher/win32/main_win32.cpp");

  auto fury = add_executable(project, "fury");
  add_source_file(fury, "demos/squares_of_fury/game.cpp");
  link_with(fury, launcher);

  return true;
}
