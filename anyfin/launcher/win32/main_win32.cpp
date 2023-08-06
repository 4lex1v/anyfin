
#include "anyfin/runtime/game.hpp"
#include "anyfin/platform/win32/common_win32.hpp"

int WinMainCRTStartup () {
  auto status = game_main();
  return static_cast<int>(status.error_code);
}
