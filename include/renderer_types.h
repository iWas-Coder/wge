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
  Matrix4 model;
  geometry *geometry;
} geometry_render_data;

typedef enum {
  BUILTIN_RENDERPASS_WORLD = 0x01,
  BUILTIN_RENDERPASS_UI    = 0x02
} builtin_renderpass;

typedef struct {
  f32 delta_time;
  u32 geometry_count;
  geometry_render_data *geometries;
  u32 ui_geometry_count;
  geometry_render_data *ui_geometries;
} render_packet;

typedef struct renderer_backend {
  u64 frame_number;
  b8 (*initialize)(struct renderer_backend *backend, const char *application_name);
  void (*shutdown)(struct renderer_backend *backend);
  void (*resized)(struct renderer_backend *backend, u16 width, u16 height);
  b8 (*begin_frame)(struct renderer_backend *backend, f32 delta_time);
  void (*update_world)(Matrix4 proj,
                       Matrix4 view,
                       Vector3 view_pos,
                       Vector4 ambient_color,
                       i32 mode);
  void (*update_ui)(Matrix4 proj,
                    Matrix4 view,
                    i32 mode);
  b8 (*end_frame)(struct renderer_backend *backend, f32 delta_time);
  b8 (*begin_renderpass)(struct renderer_backend *backend, u8 renderpass_id);
  b8 (*end_renderpass)(struct renderer_backend *backend, u8 renderpass_id);
  void (*draw_geometry)(geometry_render_data data);
  b8 (*create_geometry)(geometry *geometry,
                        u32 vertex_size,
                        u32 vertex_count,
                        const void *vertices,
                        u32 index_size,
                        u32 index_count,
                        const void *indices);
  void (*destroy_geometry)(geometry *geometry);
  void (*create_texture)(const u8 *pixels, texture *texture);
  void (*destroy_texture)(texture *texture);
  b8 (*create_material)(material *material);
  void (*destroy_material)(material *material);
} renderer_backend;
