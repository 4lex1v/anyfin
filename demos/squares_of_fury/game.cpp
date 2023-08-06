
#include "anyfin/base.hpp"

#include "anyfin/runtime/runtime.hpp"

int game_main () {
  create_window_system();

  while (true) {
    pump_platform_events();
  }

  return 0;
}
