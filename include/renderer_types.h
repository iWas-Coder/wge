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

typedef enum {
  RENDERER_BACKEND_TYPE_VULKAN,
  RENDERER_BACKEND_TYPE_OPENGL,
  RENDERER_BACKEND_TYPE_DIRECTX
} renderer_backend_type;

typedef struct {
  f32 delta_time;
} render_packet;

typedef struct renderer_backend {
  struct platform_state *plat_state;
  u64 frame_number;

  b8 (*initialize)(struct renderer_backend *backend,
                   const char *application_name,
                   struct platform_state *plat_state);
  void (*shutdown)(struct renderer_backend *backend);
  void (*resized)(struct renderer_backend *backend, u16 width, u16 height);
  b8 (*begin_frame)(struct renderer_backend *backend, f32 delta_time);
  b8 (*end_frame)(struct renderer_backend *backend, f32 delta_time);
} renderer_backend;
