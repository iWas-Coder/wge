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

#include <resource_types.h>

typedef struct {
  u32 max_loader_count;
  char *asset_base_path;  // relative path
} resource_system_config;

typedef struct resource_loader {
  u32 id;
  resource_type type;
  const char *custom_type;
  const char *type_path;
  b8 (*load)(struct resource_loader *self, const char *name, resource *out_resource);
  void (*unload)(struct resource_loader *self, resource *resource);
} resource_loader;

b8 resource_system_initialize(u64 *memory_requirements,
                              void *state,
                              resource_system_config config);

void resource_system_shutdown(void *state);

KAPI b8 resource_system_register_loader(resource_loader loader);

KAPI b8 resource_system_load(const char *name,
                             resource_type type,
                             resource *out_resource);

KAPI b8 resource_system_load_custom(const char *name,
                                    const char *custom_type,
                                    resource *out_resource);

KAPI void resource_system_unload(resource *resource);

KAPI char *resource_system_get_base_path(void);
