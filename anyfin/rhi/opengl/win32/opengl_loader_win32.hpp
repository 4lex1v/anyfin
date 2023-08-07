
#pragma once

#include "anyfin/rhi/rhi.hpp"

struct Rhi_Setup_Context {
  void *app_instance; // HINSTANCE
  void *window;       // HWND
};

typedef HGLRC (*wgl_create_context_attribs_arb_t) (HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL  (*wgl_choose_pixel_format_arb_t)    (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef BOOL  (*wgl_swap_interval_ext_t)          (int interval);
