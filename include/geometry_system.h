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

#include <renderer_types.h>

#define FALLBACK_GEOMETRY_NAME "fallback"

typedef struct {
  u32 max_geometry_count;
} geometry_system_config;

typedef struct {
  u32 vertex_count;
  vertex_3d *vertices;
  u32 index_count;
  u32 *indices;
  char name[GEOMETRY_NAME_MAX_LEN];
  char material_name[MATERIAL_NAME_MAX_LEN];
} geometry_config;

b8 geometry_system_initialize(u64 *memory_requirements,
                              void *state,
                              geometry_system_config config);

void geometry_system_shutdown(void *state);

geometry *geometry_system_get(u32 id);

geometry *geometry_system_get_from_cfg(geometry_config cfg, b8 auto_release);

geometry *geometry_system_get_fallback(void);

void geometry_system_release(geometry *geometry);

geometry_config geometry_system_gen_plane_cfg(f32 width,
                                              f32 height,
                                              u32 x_segment_count,
                                              u32 y_segment_count,
                                              f32 tile_x,
                                              f32 tile_y,
                                              const char *name,
                                              const char *material_name);
