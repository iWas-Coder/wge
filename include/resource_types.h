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

#include <math_types.h>

#define TEXTURE_NAME_MAX_LEN 92  // should be [256, 512], but it SIGSEGV's
#define MATERIAL_NAME_MAX_LEN 256

typedef struct {
  u32 id;
  u32 width;
  u32 height;
  u8 channel_count;
  b8 has_transparency;
  u32 generation;
  char name[TEXTURE_NAME_MAX_LEN];
  void *data;
} texture;

typedef enum {
  TEXTURE_USE_UNKNOWN     = 0x00,
  TEXTURE_USE_MAP_DIFFUSE = 0x01
} texture_use;

typedef struct {
  texture *texture;
  texture_use use;
} texture_map;

typedef struct {
  u32 id;
  u32 generation;
  u32 internal_id;
  char name[MATERIAL_NAME_MAX_LEN];
  Vector4 diffuse_color;
  texture_map diffuse_map;
} material;
