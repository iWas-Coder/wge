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


#include <input.h>
#include <event.h>
#include <logger.h>
#include <kmemory.h>

#define KEYS_ARRAY_LENGTH 256

typedef struct {
  b8 keys[KEYS_ARRAY_LENGTH];
} keyboard_state;

typedef struct {
  i16 x;
  i16 y;
  u8 buttons[BUTTON_MAX_BUTTONS];
} mouse_state;

typedef struct {
  keyboard_state keyboard_current;
  keyboard_state keyboard_previous;
  mouse_state mouse_current;
  mouse_state mouse_previous;
} input_state;

static input_state *state_ptr;

void input_system_initialize(u64* memory_requirements, void* state) {
  *memory_requirements = sizeof(input_state);
  if (!state) return;
  kzero_memory(state, sizeof(input_state));
  state_ptr = state;
  KINFO("Input system initialized");
}

void input_system_shutdown(void *state) {
  (void) state;  // Unused parameter

  state_ptr = 0;
}

void input_update(f64 delta_time) {
  (void) delta_time;  // Unused parameter

  if (!state_ptr) return;
  kcopy_memory(&state_ptr->keyboard_previous,
               &state_ptr->keyboard_current,
               sizeof(keyboard_state));
  kcopy_memory(&state_ptr->mouse_previous,
               &state_ptr->mouse_current,
               sizeof(mouse_state));
}

void input_process_key(keys key, b8 pressed) {
  if (state_ptr->keyboard_current.keys[key] != pressed) {
    state_ptr->keyboard_current.keys[key] = pressed;
    event_context ctx;
    ctx.data.u16[0] = key;
    event_fire(pressed ? EVENT_CODE_KEY_PRESSED : EVENT_CODE_KEY_RELEASED, 0, ctx);
  }
}

b8 input_is_key_down(keys key) {
  if (!state_ptr) return false;
  return state_ptr->keyboard_current.keys[key] == true;
}

b8 input_is_key_up(keys key) {
  if (!state_ptr) return true;
  return state_ptr->keyboard_current.keys[key] == false;
}

b8 input_was_key_down(keys key) {
  if (!state_ptr) return false;
  return state_ptr->keyboard_previous.keys[key] == true;
}

b8 input_was_key_up(keys key) {
  if (!state_ptr) return true;
  return state_ptr->keyboard_previous.keys[key] == false;
}

void input_process_button(buttons button, b8 pressed) {
  if (state_ptr->mouse_current.buttons[button] != pressed) {
    state_ptr->mouse_current.buttons[button] = pressed;
    event_context ctx;
    ctx.data.u16[0] = button;
    event_fire(pressed ? EVENT_CODE_BUTTON_PRESSED : EVENT_CODE_BUTTON_RELEASED, 0, ctx);
  }
}

void input_process_mouse_move(i16 x, i16 y) {
  if (state_ptr->mouse_current.x != x || state_ptr->mouse_current.y != y) {
    // KDEBUG("Mouse pos: (%i, %i)", x, y);
    state_ptr->mouse_current.x = x;
    state_ptr->mouse_current.y = y;
    event_context ctx;
    ctx.data.u16[0] = x;
    ctx.data.u16[1] = y;
    event_fire(EVENT_CODE_MOUSE_MOVED, 0, ctx);
  }
}

void input_process_mouse_wheel(i8 z_delta) {
  event_context ctx;
  ctx.data.u8[0] = z_delta;
  event_fire(EVENT_CODE_MOUSE_WHEEL, 0, ctx);
}

b8 input_is_button_down(buttons button) {
  if (!state_ptr) return false;
  return state_ptr->mouse_current.buttons[button] == true;
}

b8 input_is_button_up(buttons button) {
  if (!state_ptr) return true;
  return state_ptr->mouse_current.buttons[button] == false;
}

b8 input_was_button_down(buttons button) {
  if (!state_ptr) return false;
  return state_ptr->mouse_previous.buttons[button] == true;
}

b8 input_was_button_up(buttons button) {
  if (!state_ptr) return true;
  return state_ptr->mouse_previous.buttons[button] == false;
}

void input_get_mouse_position(i32 *x, i32 *y) {
  if (!state_ptr) {
    *x = *y = 0;
    return;
  }
  *x = state_ptr->mouse_current.x;
  *y = state_ptr->mouse_current.y;
}

void input_get_previous_mouse_position(i32 *x, i32 *y) {
  if (!state_ptr) {
    *x = *y = 0;
    return;
  }
  *x = state_ptr->mouse_previous.x;
  *y = state_ptr->mouse_previous.y;
}
