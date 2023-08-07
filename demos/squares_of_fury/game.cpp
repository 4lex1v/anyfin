
#include "anyfin/base.hpp"

#include "anyfin/runtime/runtime.hpp"
#include "anyfin/runtime/status_code.hpp"

#include "anyfin/window/window.hpp"

#include "anyfin/rhi/opengl/opengl.hpp"

static const usize window_width  = 1024;
static const usize window_height = 720;

Status_Code game_main () {
  use(Status_Code);
  
  create_window_system("Squares of Fury", window_width, window_height);

  glViewport(0, 0, window_width, window_height);

  while (true) {
    System_Event *events;
    usize events_count;
    if (pump_window_events(&events, &events_count)) break;

    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    check_status(present_frame());
  }

  return Success;
}

#ifdef PLATFORM_WIN32
extern "C" int _fltused = 0;
#endif
