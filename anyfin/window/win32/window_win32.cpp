
#include "anyfin/window/window.hpp"
#include "anyfin/window/events.hpp"

#include "anyfin/platform/win32/common_win32.hpp"

#ifdef RHI_OPENGL
  #include "anyfin/rhi/opengl/win32/opengl_loader_win32.hpp"
#else
  #error "Unsupported RHI backend type"
#endif

static const usize MAX_SUPPORTED_EVENTS = 256;

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

Status_Code create_window_system (const char *title, usize window_width, usize window_height) {
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
    WS_OVERLAPPEDWINDOW, //| WS_VISIBLE,
    CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height,
    nullptr, nullptr, app_instance, nullptr);

  if (!window) return get_system_error();

  Rhi_Setup_Context rhi_setup_context {
#ifdef RHI_OPENGL
    .app_instance = app_instance,
    .window       = window,
#endif
  };

  // TODO: Must communicate this issue to the user
  if (auto status = setup_rhi_backend(&rhi_setup_context); !status) {
    DestroyWindow(window);
    UnregisterClass(window_class.lpszClassName, app_instance);
    return status;
  }

  ShowWindow(window, SW_SHOW);

  return Success;
}

bool pump_window_events (System_Event **system_events, usize *events_count) {
  bool quit_requested = false;

  static System_Event buffered_events[MAX_SUPPORTED_EVENTS];
  usize count = 0;

  MSG message;
  while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
    switch (message.message) {
      case WM_QUIT: {
        quit_requested = true;
        break;
      }

      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP:
      case WM_KEYDOWN:
      case WM_KEYUP: {
        auto &event = buffered_events[count++];
        event.tag = System_Event::Keyboard;

        auto &input = event.keyboard;

        input.key_code = static_cast<u8>(message.wParam);

        // {
        //   // INFO: KeyState is a SHORT that when pressed sets the upper bit to 1
        //   // https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate
        //   const u16 shift   = (GetKeyState(VK_SHIFT) & (1 << 15)) >> 15;
        //   const u16 alt     = (GetKeyState(VK_MENU) & (1 << 15)) >> 15; 
        //   const u16 control = (GetKeyState(VK_CONTROL) & (1 << 15)) >> 15;

        //   input.modifiers = pack_modifiers(control, shift, alt);
        // }

        // {
        //   const u32 was_down = ((message.lParam & (1 << 30)) != 0);
        //   const u32 is_down  = ((message.lParam & (1ul << 31)) == 0);
        //   input.press_state = pack_press_state(was_down, is_down);
        // }
          
        break;
      }

      default: {
        DispatchMessage(&message);
        break;
      }
    }
  }

  *system_events = buffered_events;
  *events_count  = count;

  return quit_requested;
}

Status_Code present_frame () {
#ifdef RHI_OPENGL
  if (!SwapBuffers(wglGetCurrentDC())) return get_system_error();
#else
  #error "Unsupported RHI backend"
#endif
  return Status_Code::Success;
}
