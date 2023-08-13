
#include "anyfin/base.hpp"

#include "anyfin/core/assert.hpp"
#include "anyfin/core/prelude.hpp"
#include "anyfin/core/status_code.hpp"

#include "anyfin/platform/win32/common_win32.hpp"
#include <wingdi.h> // Excluded by default 

#define OPENGL_LOADER
#include "anyfin/rhi/opengl/opengl.hpp"
#include "anyfin/rhi/opengl/win32/opengl_loader_win32.hpp"

#define WGL_DRAW_TO_WINDOW_ARB           0x2001
#define WGL_ACCELERATION_ARB             0x2003
#define WGL_SUPPORT_OPENGL_ARB           0x2010
#define WGL_DOUBLE_BUFFER_ARB            0x2011
#define WGL_PIXEL_TYPE_ARB               0x2013
#define WGL_TYPE_RGBA_ARB                0x202B
#define WGL_FULL_ACCELERATION_ARB        0x2027
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20A9
#define WGL_RED_BITS_ARB                 0x2015
#define WGL_GREEN_BITS_ARB               0x2017
#define WGL_BLUE_BITS_ARB                0x2019
#define WGL_ALPHA_BITS_ARB               0x201B
#define WGL_DEPTH_BITS_ARB               0x2022
#define WGL_SWAP_METHOD_ARB              0x2007
#define WGL_SWAP_EXCHANGE_ARB            0x2028
#define WGL_COLOR_BITS_ARB               0x2014
#define WGL_STENCIL_BITS_ARB             0x2023
#define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB               0x2093
#define WGL_CONTEXT_FLAGS_ARB                     0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB                 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

bool gl_check_errors ();

static wgl_choose_pixel_format_arb_t    wglChoosePixelFormatARB;

wgl_swap_interval_ext_t          wglSwapIntervalEXT;
wgl_create_context_attribs_arb_t wglCreateContextAttribsARB;

// NOTE:
//   - https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)
static void setup_window_pixel_format (const HDC dc) {
  int suggested_pixel_format_index = 0;
  GLuint extended_pick = 0;

  if (wglChoosePixelFormatARB) {
    const int attribs[] = {
      WGL_DRAW_TO_WINDOW_ARB,  GL_TRUE,
      WGL_ACCELERATION_ARB,    WGL_FULL_ACCELERATION_ARB,
      WGL_SUPPORT_OPENGL_ARB,  GL_TRUE,
      WGL_DOUBLE_BUFFER_ARB,   GL_TRUE,
      WGL_SWAP_METHOD_ARB,     WGL_SWAP_EXCHANGE_ARB,
      WGL_PIXEL_TYPE_ARB,      WGL_TYPE_RGBA_ARB,
      WGL_COLOR_BITS_ARB,      32,
      WGL_DEPTH_BITS_ARB,      24,
      WGL_STENCIL_BITS_ARB,    8,
      0,
    };

    wglChoosePixelFormatARB(dc, attribs, NULL, 1, &suggested_pixel_format_index, &extended_pick);
  }

  if (!extended_pick) {
    PIXELFORMATDESCRIPTOR desired_pixel_format = {0};
    desired_pixel_format.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    desired_pixel_format.nVersion   = 1;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    desired_pixel_format.cColorBits = 32;
    desired_pixel_format.cDepthBits = 24;
    desired_pixel_format.cAlphaBits = 8;
    desired_pixel_format.iLayerType = PFD_MAIN_PLANE;

    suggested_pixel_format_index = ChoosePixelFormat(dc, &desired_pixel_format);
  }
  
  PIXELFORMATDESCRIPTOR suggested_pixel_format;
  DescribePixelFormat(dc, suggested_pixel_format_index, sizeof(suggested_pixel_format), &suggested_pixel_format);
  SetPixelFormat(dc, suggested_pixel_format_index, &suggested_pixel_format);
}

Status_Code setup_rhi_backend (Rhi_Setup_Context *context) {
  use(Status_Code);

  auto app_instance = reinterpret_cast<HINSTANCE>(context->app_instance);
  auto window       = reinterpret_cast<HWND>(context->window);
  
  WNDCLASS dummy_window_class {
    .style         = CS_OWNDC,
    .lpfnWndProc   = DefWindowProcA,
    .hInstance     = app_instance,
    .lpszClassName = TEXT("wgl_loader"),
  };

  if (!RegisterClass(&dummy_window_class)) return get_system_error();

  auto dummy_window = CreateWindowEx(
    0,
    dummy_window_class.lpszClassName,
    TEXT("wgl_loader"),
    0,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
    nullptr, nullptr,
    dummy_window_class.hInstance,
    nullptr);

  auto dummy_dc = GetDC(dummy_window);

  setup_window_pixel_format(dummy_dc);

  HGLRC opengl_runtime_context = wglCreateContext(dummy_dc);
  if (!wglMakeCurrent(dummy_dc, opengl_runtime_context)) return get_system_error();

  wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb_t) wglGetProcAddress("wglChoosePixelFormatARB");
  assert(wglChoosePixelFormatARB);

  wglCreateContextAttribsARB = (wgl_create_context_attribs_arb_t) wglGetProcAddress("wglCreateContextAttribsARB");
  assert(wglCreateContextAttribsARB);

  wglSwapIntervalEXT = (wgl_swap_interval_ext_t) wglGetProcAddress("wglSwapIntervalEXT");
  assert(wglSwapIntervalEXT);

  auto module = LoadLibrary(TEXT("opengl32.dll"));
  defer { FreeLibrary(module); };

#define link_opengl_proc(NAME)                                          \
  do {                                                                  \
    NAME = (type_##NAME *) wglGetProcAddress(#NAME);                    \
    if ((NAME == nullptr) ||                                            \
        (NAME == (void*)0x1) ||                                         \
        (NAME == (void*)0x2) ||                                         \
        (NAME == (void*)0x3) ||                                         \
        (NAME == (void*)-1)) [[unlikely]] {                             \
      NAME = (type_##NAME *) GetProcAddress(module, #NAME);             \
    }                                                                   \
    assert_msg(NAME, "Couldn't link OpenGL function '" stringify(NAME) "'"); \
  } while(0)

  link_opengl_proc(glActiveTexture);
  link_opengl_proc(glAttachShader);
  link_opengl_proc(glBindBuffer);
  link_opengl_proc(glBindVertexArray);
  link_opengl_proc(glBufferData);
  link_opengl_proc(glBufferSubData);
  link_opengl_proc(glCompileShader);
  link_opengl_proc(glCreateProgram);
  link_opengl_proc(glCreateShader);
  link_opengl_proc(glDebugMessageCallback);
  link_opengl_proc(glDebugMessageControl);
  link_opengl_proc(glDeleteBuffers);
  link_opengl_proc(glDeleteProgram);
  link_opengl_proc(glDeleteShader);
  link_opengl_proc(glDeleteVertexArrays);
  link_opengl_proc(glEnableVertexAttribArray);
  link_opengl_proc(glGenBuffers);
  link_opengl_proc(glGenVertexArrays);
  link_opengl_proc(glGetProgramiv);
  link_opengl_proc(glGetShaderInfoLog);
  link_opengl_proc(glGetProgramInfoLog);
  link_opengl_proc(glGetShaderiv);
  link_opengl_proc(glGetUniformLocation);
  link_opengl_proc(glLinkProgram);
  link_opengl_proc(glShaderSource);
  link_opengl_proc(glUniform1i);
  link_opengl_proc(glUniform2ui);
  link_opengl_proc(glUniform2uiv);
  link_opengl_proc(glUniform3f);
  link_opengl_proc(glUniform3fv);
  link_opengl_proc(glUniform4f);
  link_opengl_proc(glUniform4fv);
  link_opengl_proc(glUniformMatrix2fv);
  link_opengl_proc(glUniformMatrix3fv);
  link_opengl_proc(glUniformMatrix4fv);
  link_opengl_proc(glUseProgram);
  link_opengl_proc(glVertexAttribPointer);
  link_opengl_proc(glTexStorage2D);

  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(opengl_runtime_context);
  ReleaseDC(dummy_window, dummy_dc);
  DestroyWindow(dummy_window);

  const auto dc = GetDC(window);
  setup_window_pixel_format(dc);

  static int opengl_attributes [] {
    WGL_CONTEXT_MAJOR_VERSION_ARB,  4,
    WGL_CONTEXT_MINOR_VERSION_ARB,  6,
    WGL_CONTEXT_FLAGS_ARB,          WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
    WGL_CONTEXT_PROFILE_MASK_ARB,   WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
  };

  const auto opengl_runtime_render_device = wglCreateContextAttribsARB(dc, nullptr, opengl_attributes);
  if (!opengl_runtime_render_device) {
    return Status_Code { Status_Code::Fatal_Error, "Failed to create OpenGL render device" };
  }

  if (!wglMakeCurrent(dc, opengl_runtime_render_device)) {
    return Status_Code { Status_Code::Fatal_Error, "Failed to create OpenGL render device" };
  }

  return Success;
}

