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

#include <input.h>
#include <event.h>
#include <logger.h>
#include <darray.h>

#include <windows.h>
#include <windowsx.h>

#include <vulkan_types.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#define WINDOW_CLASS "wge_window_class"

typedef struct {
  HINSTANCE h_instance;
  HWND hwnd;
  VkSurfaceKHR surface;
  f64 clock_frequency;
  LARGE_INTEGER start_time;
} platform_state;

static platform_state *state_ptr;

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
  switch (msg) {
  case WM_ERASEKGND:
    // TODO: Notify the OS that erasing will be handled by the application to prevent flicker
    return 1;
  case WM_CLOSE:
    event_fire(EVENT_CODE_APPLICATION_QUIT, 0, (event_context) {0});
    return true;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_SIZE: {
    RECT r;
    GetClientRect(hwnd, &r);
    u32 width = r.right - r.left;
    u32 height = r.bottom - r.top;
    event_context context = {
      .data.u16 = { (u16) width, (u16) height }
    };
    event_fire(EVENT_CODE_RESIZED, 0, context);
  } break;
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP: {
    b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
    keys key = (u16) w_param;

    // ALT is being pressed
    if (w_param == VK_MENU) {
      if (GetKeyState(VK_RMENU) & 0x8000) key = KEY_RALT;
      else if (GetKeyState(VK_LMENU) & 0x8000) key = KEY_LALT;
    }
    // SHIFT is being pressed
    else if (w_param == VK_SHIFT) {
      if (GetKeyState(VK_RSHIFT) & 0x8000) key = KEY_RSHIFT;
      else if (GetKeyState(VK_LSHIFT) & 0x8000) key = KEY_LSHIFT;
    }
    // CONTROL is being pressed
    else if (w_param == VK_CONTROL) {
      if (GetKeyState(VK_RCONTROL) & 0x8000) key = KEY_RCONTROL;
      else if (GetKeyState(VK_LCONTROL) & 0x8000) key = KEY_LCONTROL;
    }

    input_process_key(key, pressed);
  } break;
  case WM_MOUSEMOVE: {
    i32 x_position = GET_X_LPARAM(l_param);
    i32 y_position = GET_Y_LPARAM(l_param);
    input_process_mouse_move(x_position, y_position);
  } break;
  case WM_MOUSEWHEEL: {
    i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
    if (z_delta) {
      z_delta = (z_delta < 0) ? -1 : 1;
      input_process_mouse_wheel(z_delta);
    }
  } break;
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP: {
    b8 pressed = (msg == WM_LBUTTONDOWN ||
                  msg == WM_MBUTTONDOWN ||
                  msg == WM_RBUTTONDOWN);
    buttons mouse_button = BUTTON_MAX_BUTTONS;
    switch (msg) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      mouse_button = BUTTON_LEFT;
      break;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      mouse_button = BUTTON_MIDDLE;
      break;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      mouse_button = BUTTON_RIGHT;
      break;
    }
    if (mouse_button != BUTTON_MAX_BUTTONS) input_process_button(mouse_button, pressed);
  } break;
  }
  return DefWindowProcA(hwnd, msg, w_param, l_param);
}

b8 platform_system_startup(u64 *memory_requirements,
                           void *state,
                           const char *application_name,
                           i32 x,
                           i32 y,
                           i32 width,
                           i32 height) {
  *memory_requirements = sizeof(platform_state);
  if (!state) return true;
  state_ptr = state;
  state_ptr->h_instance = GetModuleHandleA(0);

  // Window class
  HICON icon = LoadIcon(state_ptr->h_instance, IDI_APPLICATION);
  WNDCLASSA wc;
  memset(&wc, 0, sizeof(wc));
  wc.style = CS_DBLCLKS;
  wc.lpfnWndProc = win32_process_message;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = state_ptr->h_instance;
  wc.hIcon = icon;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszClassName = WINDOW_CLASS;

  if (!RegisterClassA(&wc)) {
    MessageBoxA(NULL, "Window registration failed", "ERROR", MB_ICONEXCLAMATION | MB_OK);
    KFATAL("Window registration failed");
    return false;
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
  HWND handle = CreateWindowExA(window_ex_style, WINDOW_CLASS, application_name, window_style, window_x, window_y, window_width, window_height, 0, 0, state_ptr->h_instance, 0);

  if (!handle) {
    MessageBoxA(NULL, "Window creation failed", "ERROR", MB_ICONEXCLAMATION | MB_OK);
    KFATAL("Window creation failed");
    return false;
  }
  else state_ptr->hwnd = handle;

  // Show window
  // TODO: if the window should not accept input, this should be false.
  b32 should_activate = 1;
  i32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
  ShowWindow(state_ptr->hwnd, show_window_command_flags);

  // Clock
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  state_ptr->clock_frequency = 1.0f / (f64) frequency.QuadPart;
  QueryPerformanceCounter(&state_ptr->start_time);

  return true
    }

void platform_system_shutdown(void *plat_state) {
  if (!state_ptr || !state_ptr->hwnd) return;
  DestroyWindow(state_ptr->hwnd);
  state_ptr->hwnd = 0;
}

b8 platform_pump_messages(void) {
  if (!state_ptr) return true;
  MSG message;
  while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }
  return true;
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

void platform_console_write_loglevel(const char *message, u8 color, DWORD handle) {
  HANDLE console_handle = GetStdHandle(handle);
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
  u64 length = kstrlen(message);
  LPDWORD number_written = 0;
  WriteConsoleA(GetStdHandle(handle), message, (DWORD) length, number_written, 0);
}

void platform_console_write(const char *message, u8 color) {
  platform_console_write_loglevel(message, color, STD_OUTPUT_HANDLE);
}

void platform_console_write_error(const char *message, u8 color) {
  platform_console_write_loglevel(message, color, STD_ERROR_HANDLE);
}

f64 platform_get_absolute_time(void) {
  if (!state_ptr) return 0;
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  return (f64) now.QuadPart * state_ptr->clock_frequency;
}

void platform_sleep(u64 ms) {
  Sleep(ms);
}

void platform_get_required_extension_names(const char ***names_darray) {
  darray_push(*names_darray, &"VK_KHR_win32_surface");
}

b8 platform_create_vulkan_surface(vulkan_context *context) {
  if (!state_ptr) return false;

  VkWin32SurfaceCreateInfoKHR create_info = {
    .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
    .hinstance = state_ptr->h_instance,
    .hwnd = state_ptr->hwnd
  };
  VkResult result = vkCreateWin32SurfaceKHR(context->instance,
                                            &create_info,
                                            context->allocator,
                                            &state_ptr->surface);
  if (result != VK_SUCCESS) {
    KFATAL("Failed to create surface");
    return false;
  }
  context->surface = state_ptr->surface;
  return true;
}

#endif  // KPLATFORM_WINDOWS
