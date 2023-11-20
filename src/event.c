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


#include <event.h>
#include <darray.h>
#include <kmemory.h>

#define MAX_MESSAGE_CODES 1024 * 16

typedef struct {
  void *listener;
  PFN_on_event callback;
} registered_event;

typedef struct {
  registered_event *events;
} event_code_entry;

typedef struct {
  event_code_entry registered[MAX_MESSAGE_CODES];
} event_system_state;

static event_system_state *state_ptr;

void event_system_initialize(u64 *memory_requirements, void *state) {
  *memory_requirements = sizeof(event_system_state);
  if (!state) return;
  kzero_memory(state, sizeof(state));
  state_ptr = state;
}

void event_system_shutdown(void *state) {
  (void) state;  // Unused parameter

  if (!state_ptr) return;
  for (u16 i = 0; i < MAX_MESSAGE_CODES; ++i) {
    if (state_ptr->registered[i].events) {
      darray_destroy(state_ptr->registered[i].events);
      state_ptr->registered[i].events = 0;
    }
  }
  state_ptr = 0;
}

b8 event_register(u16 code, void *listener, PFN_on_event on_event) {
  if (!state_ptr) return false;

  if (!state_ptr->registered[code].events) {
    state_ptr->registered[code].events = darray_create(registered_event);
  }

  u64 n_registered = darray_length(state_ptr->registered[code].events);
  for (u64 i = 0; i < n_registered; ++i) {
    if (state_ptr->registered[code].events[i].listener == listener) {
      // TODO: throw warning
      return false;
    }
  }

  registered_event event;
  event.listener = listener;
  event.callback = on_event;
  darray_push(state_ptr->registered[code].events, event);
  return true;
}

b8 event_unregister(u16 code, void *listener, PFN_on_event on_event) {
  if (!state_ptr) return false;

  if (!state_ptr->registered[code].events) {
    // TODO: throw warning
    return false;
  }

  u64 n_registered = darray_length(state_ptr->registered[code].events);
  for (u64 i = 0; i < n_registered; ++i) {
    registered_event e = state_ptr->registered[code].events[i];
    if (e.listener == listener && e.callback == on_event) {
      registered_event popped_e;
      darray_pop_at(state_ptr->registered[code].events, i, &popped_e);
      return true;
    }
  }
  return false;
}

b8 event_fire(u16 code, void *sender, event_context context) {
  if (!state_ptr) return false;

  if (!state_ptr->registered[code].events) {
    // TODO: throw warning
    return false;
  }

  u64 n_registered = darray_length(state_ptr->registered[code].events);
  for (u64 i = 0; i < n_registered; ++i) {
    registered_event e = state_ptr->registered[code].events[i];
    if (e.callback(code, sender, e.listener, context)) return true;
  }
  return false;
}
