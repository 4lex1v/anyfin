
#include "anyfin/window/window.hpp"

#include "anyfin/platform/win32/common_win32.hpp"

static LRESULT CALLBACK window_events_handler (HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
  LRESULT result = 0;

  switch (message) {
    // TODO: #window #ux proper support for windows closing experience
    // if (MessageBoxA(window, "Unsafed changes", "The Editor", MB_YESNOCANCEL) == IDYES) {}
    case WM_CLOSE: {
      // ShowWindow(window, SW_HIDE);
      // win32_event_bus_push_event(System_Event(Quit_Input_Event()));
      break;
    }
    // TODO: #window #ux proper support for window destruction events
    case WM_DESTROY: {break;}

    case WM_ERASEBKGND: {
      // HDC hdc = (HDC) wParam;
      // RECT client = {0};
      // GetClientRect(window, &client);
      // constexpr f32 bg_color[] { 0.0784f, 0.1529f, 0.1961f };
      
      // const u32 red   = static_cast<u32>(bg_color[0] * 255);
      // const u32 green = static_cast<u32>(bg_color[1] * 255);
      // const u32 blue  = static_cast<u32>(bg_color[2] * 255);

      // FillRect(hdc, &client, CreateSolidBrush(RGB(red, green, blue)));
      break;
    }

    case WM_DPICHANGED: {
      // Window's spec says that these two are always equal
      // ref: https://docs.microsoft.com/en-us/windows/win32/hidpi/wm-dpichanged
      // const u16 horizontal_dpi = HIWORD(wParam);
      // const u16 vertical_dpi   = LOWORD(wParam);

      // LPRECT rect = (LPRECT) lParam;

      // const auto scaled_width  = static_cast<u16>(rect->right - rect->left);
      // const auto scaled_height = static_cast<u16>(rect->bottom - rect->top);

      // win32_event_bus_push_event(Window_Scale_Input_Event {
      //   .horizontal_dpi = horizontal_dpi,
      //   .vertical_dpi   = vertical_dpi,
      //   .scaled_width   = scaled_width,
      //   .scaled_height  = scaled_height,
      // });

      // SetWindowPos(window, nullptr, rect->left, rect->top, scaled_width, scaled_height, SWP_NOZORDER | SWP_NOACTIVATE);

      break;
    }

    case WM_SIZE: {
      // RECT client;
      // GetClientRect(window, &client);

      // assert(client.top == 0 && client.left == 0);
      // win32_event_bus_push_event(System_Event(Window_Resize_Input_Event {
      //   .window_width  = static_cast<u16>(client.right - client.left),
      //   .window_height = static_cast<u16>(client.bottom - client.top),
      // }));

      break;
    }

    default: {
      result = DefWindowProc(window, message, wParam, lParam);
    } break;
  }

  return result;
}

Status_Code create_window_system (const char *title) {
  use(Status_Code);
  
  const auto app_instance = GetModuleHandle(nullptr);

  const WNDCLASS window_class {
    .style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
    .lpfnWndProc   = window_events_handler,
    .hInstance     = app_instance,
    .hCursor       = LoadCursor(nullptr, IDC_ARROW),
    .lpszClassName = TEXT("MainWindowClazz"),
  };

  if (!RegisterClass(&window_class)) return get_system_error();

  auto window = CreateWindowEx(
    0, // WS_EX_TOPMOST|WS_EX_LAYERED,
    window_class.lpszClassName,
    title,
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    CW_USEDEFAULT, CW_USEDEFAULT, 1024, 720,
    nullptr, nullptr, app_instance, nullptr);

  if (!window) get_system_error();

  return Success;
}

bool pump_window_events () {
  bool quit_requested = false;

  MSG message;
  while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
    switch (message.message) {
      case WM_QUIT: {
        quit_requested = true;
        break;
      }
      default: {
        DispatchMessage(&message);
        break;
      }
    }
  }

  return quit_requested;
}
