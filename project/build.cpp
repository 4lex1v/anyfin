
#include "cbuild.h"

extern "C" bool setup_project (const Arguments *args, Project *project) {
  add_global_compiler_options(project,
                              "-std=c++20",
                              "-O0 -g -gcodeview -DDEBUG_BUILD",
                              "-DPLATFORM_WIN32 -DPLATFORM_X64",
                              "-DRHI_OPENGL");

  add_global_include_search_path(project, ".");

  set_toolchain(project, Toolchain_Type_LLVM);

  auto opengl_rhi = add_static_library(project, "opengl_rhi");
  {
    add_source_file(opengl_rhi, "anyfin/rhi/opengl/win32/opengl_loader_win32.cpp");
    add_compiler_options(opengl_rhi, "-fno-exceptions");
  }

  auto window = add_static_library(project, "window");
  {
    add_source_file(window, "anyfin/window/win32/window_win32.cpp");
    add_compiler_options(window, "-fno-exceptions");
  }

  auto platform = add_static_library(project, "platform");
  {
    add_all_sources_from_directory(platform, "anyfin/platform/win32", "cpp", false);
    add_compiler_options(platform, "-fno-exceptions");
  }

  auto launcher = add_static_library(project, "launcher");
  {
    add_source_file(launcher, "anyfin/launcher/win32/main_win32.cpp");
    add_compiler_options(launcher, "-fno-exceptions");
  }

  auto fury = add_executable(project, "fury");
  {
    add_source_file(fury, "demos/squares_of_fury/game.cpp");
    add_compiler_options(fury, "-fno-exceptions");
    add_linker_option(fury, "/debug:full /subsystem:Windows");
    link_with(fury,
              window, launcher,
              opengl_rhi, "gdi32.lib", "opengl32.lib",
              platform,"kernel32.lib user32.lib");
  }

  return true;
}
