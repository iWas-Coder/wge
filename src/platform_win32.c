/*
* GNU WGE --- Wildebeest Game Engine™
* Copyright (C) 2023 Wasym A. Alonso
*
* This file is part of WGE.
*
* WGE is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* WGE is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with WGE.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <platform.h>

#if KPLATFORM_WINDOWS

#include <logger.h>

#include <windows.h>
#include <windowsx.h>

#define WINDOW_CLASS "wge_window_class"

typedef struct {
  HINSTANCE h_instance;
  HWND hwnd;
} internal_state;

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);
LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
  switch (msg) {
  case WM_ERASEKGND:
    // TODO: Notify the OS that erasing will be handled by the application to prevent flicker
    return 1;
  case WM_CLOSE:
    // TODO: Fire an event for the application to quit
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_SIZE: {
    // TODO: Fire an event for window resize
  } break;
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP: {
    // TODO: input processing
  } break;
  case WM_MOUSEMOVE: {
    // TODO: input processing
  } break;
  case WM_MOUSEWHEEL: {
    // TODO: input processing
  } break;
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP: {
    // TODO: input processing
  } break;
  }
  return DefWindowProcA(hwnd, msg, w_param, l_param);
}

b8 platform_startup(platform_state *plat_state, const char *application_name, i32 x, i32 y, i32 width, i32 height) {
  plat_state->internal_state = malloc(sizeof(internal_state));
  internal_state *state = (internal_state *) plat_state->internal_state;

  state->h_instance = GetModuleHandleA(0);

  // Window class
  HICON icon = LoadIcon(state->h_instance, IDI_APPLICATION);
  WNDCLASSA wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = win32_process_message;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = state->h_instance;
  wc.hIcon = icon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszClassName = WINDOW_CLASS;

  if (!RegisterClassA(&wc)) {
    MessageBoxA(NULL, "Window registration failed", "ERROR", MB_ICONEXCLAMATION | MB_OK);
    KFATAL("Window registration failed");
    return FALSE;
  }

  // Client area
  u32 client_x = x;
  u32 client_y = y;
  u32 client_width = width;
  u32 client_height = height;

  // Window area
  u32 window_x = client_x;
  u32 window_y = client_y;
  u32 window_width = client_width;
  u32 window_height = client_height;

  // Styles
  u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  u32 window_ex_style = WS_EX_APPWINDOW;
  window_style |= WS_MAXIMIZEBOX;
  window_style |= WS_MINIMIZEBOX;
  window_style |= WS_THICKFRAME;

  // Get border size
  RECT border_rect = {0, 0, 0, 0};
  AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

  // Adjust window size by border size
  window_x += border_rect.left;
  window_y += border_rect.top;
  window_width += border_rect.right - border_rect.left;
  window_height += border_rect.bottom - border_rect.top;

  // Create window
  HWND handle = CreateWindowExA(window_ex_style, WINDOW_CLASS, application_name, window_style, window_x, window_y, window_width, window_height, 0, 0, state->h_instance, 0);

  if (!handle) {
    MessageBoxA(NULL, "Window creation failed", "ERROR", MB_ICONEXCLAMATION | MB_OK);
    KFATAL("Window creation failed");
    return FALSE;
  }
  else state->hwnd = handle;

  // Show window
  // TODO: if the window should not accept input, this should be false.
  b32 should_activate = 1;
  i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
  ShowWindow(state->hwnd, show_window_command_flags);

  // Clock
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  clock_frequency = 1.0f / (f64) frequency.QuadPart;
  QueryPerformanceCounter(&start_time);

  return TRUE
    }

void platform_shutdown(platform_state *plat_state) {
  internal_state *state = (internal_state *) plat_state->internal_state;
  if (state->hwnd) {
    DestroyWindow(state->hwnd);
    state->hwnd = 0;
  }
}

b8 platform_pump_messages(platform_state *plat_state) {
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  return TRUE;
}

void *platform_allocate(u64 size, b8 aligned) {
  return malloc(size);
}

void platform_free(void *block, b8 aligned) {
  free(block);
}

void *platform_zero_memory(void *block, u64 size) {
  return memset(block, 0, size);
}

void *platform_copy_memory(void *dest, const void *source, u64 size) {
  return memcpy(dest, source, size);
}

void *platform_set_memory(void *dest, i32 value, u64 size) {
  return memset(dest, value, size);
}

void platform_console_write(const char *message, u8 color) {
  HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  static u8 levels[] = {
    64,  // FATAL
    4,   // ERROR
    6,   // WARN
    2,   // INFO
    1,   // DEBUG
    8    // TRACE
  };
  SetConsoleTextAttribute(console_handle, levels[color]);
  OutputDebugStringA(message);
  u64 length = strlen(message);
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD) length, number_written, 0);
}

void platform_console_write_error(const char *message, u8 color) {
  HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
  static u8 levels[] = {
    64,  // FATAL
    4,   // ERROR
    6,   // WARN
    2,   // INFO
    1,   // DEBUG
    8    // TRACE
  };
  SetConsoleTextAttribute(console_handle, levels[color]);
  OutputDebugStringA(message);
  u64 length = strlen(message);
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD) length, number_written, 0);
}

f64 platform_get_absolute_time(void) {
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  return (f64) now.QuadPart * clock_frequency;
}

void platform_sleep(u64 ms) {
  Sleep(ms);
}

#endif  // KPLATFORM_WINDOWS
