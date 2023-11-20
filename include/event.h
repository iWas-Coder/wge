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


#pragma once

#include <defines.h>

#define N_64_IN_128 128/64
#define N_32_IN_128 128/32
#define N_16_IN_128 128/16
#define N_8_IN_128  128/8

typedef struct {
  union {
    u8 u8[N_8_IN_128];
    i8 i8[N_8_IN_128];
    char c[N_8_IN_128];

    u16 u16[N_16_IN_128];
    i16 i16[N_16_IN_128];

    u32 u32[N_32_IN_128];
    i32 i32[N_32_IN_128];
    f32 f32[N_32_IN_128];

    u64 u64[N_64_IN_128];
    i64 i64[N_64_IN_128];
    f64 f64[N_64_IN_128];
  } data;
} event_context;

typedef enum {
  EVENT_CODE_APPLICATION_QUIT = 0x01,
  EVENT_CODE_KEY_PRESSED = 0x02,
  EVENT_CODE_KEY_RELEASED = 0x03,
  EVENT_CODE_BUTTON_PRESSED = 0x04,
  EVENT_CODE_BUTTON_RELEASED = 0x05,
  EVENT_CODE_MOUSE_MOVED = 0x06,
  EVENT_CODE_MOUSE_WHEEL = 0x07,
  EVENT_CODE_RESIZED = 0x08,
  // Event that marks the end of enum
  MAX_EVENT_CODE = 0xFF
} system_event_code;

typedef b8 (*PFN_on_event)(u16 code,
                           void *sender,
                           void *listener_inst,
                           event_context data);

void event_system_initialize(u64 *memory_requirements, void *state);
void event_system_shutdown(void *state);

KAPI b8 event_register(u16 code, void *listener, PFN_on_event on_event);
KAPI b8 event_unregister(u16 code, void *listener, PFN_on_event on_event);
KAPI b8 event_fire(u16 code, void *sender, event_context context);
