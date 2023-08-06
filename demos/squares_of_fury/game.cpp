
#include "anyfin/base.hpp"

#include "anyfin/runtime/runtime.hpp"
#include "anyfin/runtime/status_code.hpp"

#include "anyfin/window/window.hpp"

Status_Code game_main () {
  use(Status_Code);
  
  create_window_system("Squares of Fury");

  while (true) {
    if (pump_window_events()) break;
  }

  return Success;
}
