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
#include <resource_types.h>

#define FALLBACK_MATERIAL_NAME "fallback"

typedef struct {
  u32 max_material_count;
} material_system_config;

typedef struct {
  char name[MATERIAL_NAME_MAX_LEN];
  b8 auto_release;
  Vector4 diffuse_color;
  char diffuse_map_name[TEXTURE_NAME_MAX_LEN];
} material_config;

b8 material_system_initialize(u64 *memory_requirements,
                              void *state,
                              material_system_config config);

void material_system_shutdown(void *state);

material *material_system_get(const char *name);

material *material_system_get_from_cfg(material_config cfg);

void material_system_release(const char *name);
