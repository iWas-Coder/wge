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
#include <math_types.h>
#include <resource_types.h>

#define GEOMETRY_RENDER_DATA_N_TEXTURES 16

typedef enum {
  RENDERER_BACKEND_TYPE_VULKAN,
  RENDERER_BACKEND_TYPE_OPENGL,
  RENDERER_BACKEND_TYPE_DIRECTX
} renderer_backend_type;

typedef struct {
  Matrix4 proj;
  Matrix4 view;
  Matrix4 padding0;
  Matrix4 padding1;
} global_uniform_object;

typedef struct {
  Vector4 diffuse_color;
  Vector4 padding0;
  Vector4 padding1;
  Vector4 padding2;
} local_uniform_object;

typedef struct {
  u32 object_id;
  Matrix4 model;
  texture *textures[GEOMETRY_RENDER_DATA_N_TEXTURES];
} geometry_render_data;

typedef struct {
  f32 delta_time;
} render_packet;

typedef struct renderer_backend {
  u64 frame_number;
  b8 (*initialize)(struct renderer_backend *backend, const char *application_name);
  void (*shutdown)(struct renderer_backend *backend);
  void (*resized)(struct renderer_backend *backend, u16 width, u16 height);
  b8 (*begin_frame)(struct renderer_backend *backend, f32 delta_time);
  void (*update)(Matrix4 proj, Matrix4 view, Vector3 view_pos, Vector4 ambient_color, i32 mode);
  b8 (*end_frame)(struct renderer_backend *backend, f32 delta_time);
  void (*update_object)(geometry_render_data data);
  void (*create_texture)(const char *name,
                         b8 auto_release,
                         i32 width,
                         i32 height,
                         i32 channel_count,
                         const u8 *pixels,
                         b8 has_transparency,
                         texture *out_texture);
  void (*destroy_texture)(texture *texture);
} renderer_backend;
