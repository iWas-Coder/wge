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

#define FALLBACK_TEXTURE_NAME "fallback"

typedef struct {
  u32 max_texture_count;
} texture_system_config;

b8 texture_system_initialize(u64 *memory_requirements,
                             void *state,
                             texture_system_config config);

void texture_system_shutdown(void *state);

texture *texture_system_get(const char *name, b8 auto_release);

texture *texture_system_get_fallback(void);

void texture_system_release(const char *name);
