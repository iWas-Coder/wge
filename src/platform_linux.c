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

#if KPLATFORM_LINUX

#include <logger.h>

#include <xcb/xcb.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xlib-xcb.h>

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>  // nanosleep
#else
#include <unistd.h>  // usleep
#endif  // _POSIX_C_SOURCE >= 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIME_NS_IN_S 1e-9
#define TIME_MS_IN_S 1e3

typedef struct {
  Display *display;
  xcb_connection_t *connection;
  xcb_window_t window;
  xcb_screen_t *screen;
  xcb_atom_t wm_protocols;
  xcb_atom_t wm_delete_win;
} internal_state;

b8 platform_startup(platform_state *plat_state, const char *application_name, i32 x, i32 y, i32 width, i32 height) {
  plat_state->internal_state = malloc(sizeof(internal_state));
  internal_state *state = (internal_state *) plat_state->internal_state;

  // Connect to X
  state->display = XOpenDisplay(NULL);
  // Get display's connection
  state->connection = XGetXCBConnection(state->display);
  if (xcb_connection_has_error(state->connection)) {
    KFATAL("Failed to connect to X server via XCB");
    return FALSE;
  }
  // Get data from X
  const struct xcb_setup_t *setup = xcb_get_setup(state->connection);

  // Iterate through screens
  xcb_screen_iterator_t i = xcb_setup_roots_iterator(setup);
  int screen_p = 0;
  for (i32 s = screen_p; s > 0; --s) xcb_screen_next(&i);
  state->screen = i.data;
  // Allocate ID for window
  state->window = xcb_generate_id(state->connection);

  // Event types
  u32 event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  u32 event_values = XCB_EVENT_MASK_BUTTON_PRESS |
    XCB_EVENT_MASK_BUTTON_RELEASE                |
    XCB_EVENT_MASK_KEY_PRESS                     |
    XCB_EVENT_MASK_KEY_RELEASE                   |
    XCB_EVENT_MASK_EXPOSURE                      |
    XCB_EVENT_MASK_POINTER_MOTION                |
    XCB_EVENT_MASK_STRUCTURE_NOTIFY;
  u32 value_list[] = {
    state->screen->black_pixel,
    event_values
  };

  // Create window
  // xcb_void_cookie_t cookie =
  xcb_create_window(state->connection,
                    XCB_COPY_FROM_PARENT,
                    state->window,
                    state->screen->root,
                    x, y, width, height, 0,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    state->screen->root_visual,
                    event_mask,
                    value_list);
  // Change title
  xcb_change_property(state->connection,
                      XCB_PROP_MODE_REPLACE,
                      state->window,
                      XCB_ATOM_WM_NAME,
                      XCB_ATOM_STRING,
                      8,  // 8-bit at a time
                      strlen(application_name),
                      application_name);
  // Notify when WM tries to destroy window
  xcb_intern_atom_cookie_t wm_delete_cookie = xcb_intern_atom(state->connection,
                                                              0,
                                                              strlen("WM_DELETE_WINDOW"),
                                                              "WM_DELETE_WINDOW");
  xcb_intern_atom_cookie_t wm_protocols_cookie = xcb_intern_atom(state->connection,
                                                                 0,
                                                                 strlen("WM_PROTOCOLS"),
                                                                 "WM_PROTOCOLS");
  xcb_intern_atom_reply_t *wm_delete_reply = xcb_intern_atom_reply(state->connection,
                                                                   wm_delete_cookie,
                                                                   NULL);
  xcb_intern_atom_reply_t *wm_protocols_reply = xcb_intern_atom_reply(state->connection,
                                                                      wm_protocols_cookie,
                                                                      NULL);
  state->wm_delete_win = wm_delete_reply->atom;
  state->wm_protocols = wm_protocols_reply->atom;

  xcb_change_property(state->connection,
                      XCB_PROP_MODE_REPLACE,
                      state->window,
                      wm_protocols_reply->atom,
                      4, 32, 1,
                      &wm_delete_reply->atom);
  // Map window to screen
  xcb_map_window(state->connection, state->window);

  // Flush stream
  i32 stream_result = xcb_flush(state->connection);
  if (stream_result <= 0) {
    KFATAL("Error when flushing the stream: %d", stream_result);
    return FALSE;
  }

  return TRUE;
}

void platform_shutdown(platform_state *plat_state) {
  internal_state *state = (internal_state *) plat_state->internal_state;
  xcb_destroy_window(state->connection, state->window);
}

b8 platform_pump_messages(platform_state *plat_state) {
  internal_state *state = (internal_state *) plat_state->internal_state;
  xcb_generic_event_t *event = NULL;
  xcb_client_message_event_t *cm;
  b8 quit_flagged = FALSE;

  while (event) {
    event = xcb_poll_for_event(state->connection);
    if (!event) break;

    // Input events
    switch (event->response_type & ~0x80) {
    case XCB_KEY_PRESS:
    case XCB_KEY_RELEASE: {
      // TODO: Key presses and releases
    } break;
    case XCB_BUTTON_PRESS:
    case XCB_BUTTON_RELEASE: {
      // TODO: Mouse button presses and releases
    } break;
    case XCB_MOTION_NOTIFY:
      // TODO: mouse movement
      break;
    case XCB_CONFIGURE_NOTIFY: {
      // TODO: Resizing
    }
    case XCB_CLIENT_MESSAGE: {
      cm = (xcb_client_message_event_t *) event;
      if (cm->data.data32[0] == state->wm_delete_win) quit_flagged = TRUE;
    } break;
    default:
      break;
    }

    free(event);
  }
  return !quit_flagged;
}

void *platform_allocate(u64 size, b8 aligned) {
  (void) aligned;  // Unused parameter
  return malloc(size);
}

void platform_free(void *block, b8 aligned) {
  (void) aligned;  // Unused parameter
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
  const char *color_strings[] = {
    "0;41",  // FATAL
    "1;31",  // ERROR
    "1;33",  // WARN
    "1;32",  // INFO
    "1;34",  // DEBUG
    "1;30"   // TRACE
  };
  printf("\033[%sm%s\033[0m", color_strings[color], message);
}

void platform_console_write_error(const char *message, u8 color) {
  const char *color_strings[] = {
    "0;41",  // FATAL
    "1;31",  // ERROR
    "1;33",  // WARN
    "1;32",  // INFO
    "1;34",  // DEBUG
    "1;30"   // TRACE
  };
  printf("\033[%sm%s\033[0m", color_strings[color], message);
}

f64 platform_get_absolute_time(void) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec + now.tv_nsec * TIME_NS_IN_S;
}

void platform_sleep(u64 ms) {
#if _POSIX_C_SOURCE >= 199309L
  struct timespec ts;
  ts.tv_sec = ms / TIME_MS_IN_S;
  ts.tv_nsec = (ms % (u64) TIME_MS_IN_S) * TIME_MS_IN_S * TIME_MS_IN_S;
  nanosleep(&ts, 0);
#else
  if (ms >= TIME_MS_IN_S) sleep(ms / TIME_MS_IN_S);
  usleep((ms % (u64) TIME_MS_IN_S) * TIME_MS_IN_S);
#endif  // _POSIX_C_SOURCE >= 199309L
}

#endif  // KPLATFORM_LINUX
