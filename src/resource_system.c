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


#include <logger.h>
#include <kstring.h>
#include <text_loader.h>
#include <image_loader.h>
#include <binary_loader.h>
#include <material_loader.h>
#include <resource_system.h>

typedef struct {
  resource_system_config config;
  resource_loader *registered_loaders;
} resource_system_state;

static resource_system_state *state_ptr = 0;

static b8 load(resource_loader *loader, const char *name, resource *out_resource) {
  if (!name || !loader || !loader->load || !out_resource) {
    out_resource->loader_id = INVALID_ID;
    return false;
  }
  out_resource->loader_id = loader->id;
  return loader->load(loader, name, out_resource);
}

b8 resource_system_initialize(u64 *memory_requirements,
                              void *state,
                              resource_system_config config) {
  if (!config.max_loader_count) {
    KFATAL("resource_system_initialize :: `max_loader_count` must be > 0");
    return false;
  }
  u64 struct_requirements = sizeof(resource_system_state);
  u64 array_requirements = sizeof(resource_loader) * config.max_loader_count;
  *memory_requirements = struct_requirements + array_requirements;
  if (!state) return true;

  state_ptr = state;
  state_ptr->config = config;
  void *array_block = (void *) ((u64 *) state + struct_requirements);
  state_ptr->registered_loaders = array_block;

  for (u32 i = 0; i < config.max_loader_count; ++i) {
    state_ptr->registered_loaders[i].id = INVALID_ID;
  }

  resource_system_register_loader(text_loader_create());
  resource_system_register_loader(binary_loader_create());
  resource_system_register_loader(image_loader_create());
  resource_system_register_loader(material_loader_create());
  
  KINFO("Resource system initialized under '%s'", config.asset_base_path);
  return true;
}

void resource_system_shutdown(void *state) {
  (void) state;  // Unused parameter
  
  if (state_ptr) state_ptr = 0;
}

b8 resource_system_register_loader(resource_loader loader) {
  if (!state_ptr) return false;
  // Ensure no loaders for the given type already exist
  for (u32 i = 0; i < state_ptr->config.max_loader_count; ++i) {
    resource_loader *l = &state_ptr->registered_loaders[i];
    if (l->id == INVALID_ID) continue;
    if (l->type == loader.type) {
      KERROR("resource_system_register_loader :: type '%d' loader already exists (ID %u)",
             loader.type,
             l->id);
      return false;
    }
    else if (loader.custom_type              &&
             kstrlen(loader.custom_type) &&
             kstrcmpi(l->custom_type, loader.custom_type)) {
      KERROR("resource_system_register_loader :: custom type '%s' loader already exists (ID %u)",
             loader.custom_type,
             l->id);
      return false;
    }
  }
  // Find an empty spot to put our loader
  for (u32 i = 0; i < state_ptr->config.max_loader_count; ++i) {
    if (state_ptr->registered_loaders[i].id != INVALID_ID) continue;
    state_ptr->registered_loaders[i] = loader;
    state_ptr->registered_loaders[i].id = i;
    KTRACE("Loader registered (ID %u)", i);
    return true;
  }
  return false;
}

b8 resource_system_load(const char *name,
                        resource_type type,
                        resource *out_resource) {
  if (state_ptr && type != RESOURCE_TYPE_CUSTOM) {
    for (u32 i = 0; i < state_ptr->config.max_loader_count; ++i) {
      resource_loader *l = &state_ptr->registered_loaders[i];
      if (l->id != INVALID_ID && l->type == type) return load(l, name, out_resource);
    }
  }
  out_resource->loader_id = INVALID_ID;
  KERROR("resource_system_load :: no loader for type '%d' was found", type);
  return false;
}

b8 resource_system_load_custom(const char *name,
                               const char *custom_type,
                               resource *out_resource) {
  if (state_ptr && custom_type && kstrlen(custom_type)) {
    for (u32 i = 0; i < state_ptr->config.max_loader_count; ++i) {
      resource_loader *l = &state_ptr->registered_loaders[i];
      if (l->id != INVALID_ID &&
          l->type == RESOURCE_TYPE_CUSTOM &&
          kstrcmpi(l->custom_type, custom_type)) return load(l, name, out_resource);
    }
  }
  out_resource->loader_id = INVALID_ID;
  KERROR("resource_system_load_custom :: no loader for type '%s' was found", custom_type);
  return false;
}

void resource_system_unload(resource *resource) {
  if (!state_ptr || !resource || resource->loader_id == INVALID_ID) return;
  resource_loader *l = &state_ptr->registered_loaders[resource->loader_id];
  if (l->id != INVALID_ID && l->unload) l->unload(l, resource);
}

char *resource_system_get_base_path(void) {
  if (!state_ptr) {
    KERROR("resource_system_get_base_path :: called before resource system init");
    return "";
  }
  return state_ptr->config.asset_base_path;
}
