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


#include <kmath.h>
#include <logger.h>
#include <kstring.h>
#include <hash_table.h>
#include <texture_system.h>
#include <material_system.h>
#include <renderer_frontend.h>

// TODO: move to resource system
#include <filesystem.h>

#define FILE_PATH_SIZE 512
#define MATERIAL_FILE_LINE_MAX_SIZE 512
#define MATERIAL_FILE_VAR_MAX_SIZE 64
#define MATERIAL_FILE_VALUE_MAX_SIZE (MATERIAL_FILE_LINE_MAX_SIZE - MATERIAL_FILE_VAR_MAX_SIZE - 2)
#define MATERIAL_FILE_EXTENSION_NAME "wmt"

typedef struct {
  material_system_config config;
  material fallback_material;
  material *registered_materials;
  hash_table registered_material_table;
} material_system_state;

typedef struct {
  u64 ref_count;
  u32 handle;
  b8 auto_release;
} material_ref;

static material_system_state *state_ptr = 0;

void destroy_material(material *m) {
  KTRACE("Destroying material '%s'...", m->name);
  if (m->diffuse_map.texture) texture_system_release(m->diffuse_map.texture->name);
  renderer_destroy_material(m);
  kzero_memory(m, sizeof(material));
  m->id = INVALID_ID;
  m->generation = INVALID_ID;
  m->internal_id = INVALID_ID;
}

b8 create_fallback_material(material_system_state *state) {
  kzero_memory(&state->fallback_material, sizeof(material));
  kstrncp(state->fallback_material.name,
          FALLBACK_MATERIAL_NAME,
          MATERIAL_NAME_MAX_LEN);
  state->fallback_material.id = INVALID_ID;
  state->fallback_material.generation = INVALID_ID;
  state->fallback_material.diffuse_color = vec4_one();  // white
  state->fallback_material.diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
  state->fallback_material.diffuse_map.texture = texture_system_get_fallback();

  if (!renderer_create_material(&state->fallback_material)) {
    KFATAL("create_fallback_material :: unable to get renderer resources for fallback material");
    return false;
  }

  return true;
}

b8 load_material(material_config cfg, material *m) {
  kzero_memory(m, sizeof(material));
  kstrncp(m->name, cfg.name, MATERIAL_NAME_MAX_LEN);
  m->diffuse_color = cfg.diffuse_color;
  // Diffuse map
  if (kstrlen(cfg.diffuse_map_name)) {
    m->diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
    m->diffuse_map.texture = texture_system_get(cfg.diffuse_map_name, true);
    if (!m->diffuse_map.texture) {
      KWARN("load_material :: unable to load texture '%s' for material '%s' (using fallback)",
            cfg.diffuse_map_name,
            m->name);
      m->diffuse_map.texture = texture_system_get_fallback();
    }
  }
  else {
    m->diffuse_map.use = TEXTURE_USE_UNKNOWN;
    m->diffuse_map.texture = 0;
  }
  // Create material
  if (!renderer_create_material(m)) {
    KERROR("load_material :: unable to get renderer resources for material '%s'", m->name);
    return false;
  }

  return true;
}

b8 load_cfg_file(const char *path, material_config *out_cfg) {
  file_handle fd;
  if (!filesystem_open(path, FILE_MODE_READ, false, &fd)) {
    KERROR("load_cfg_file :: unable to open material file for reading ('%s')", path);
    return false;
  }

  char line_buf[MATERIAL_FILE_LINE_MAX_SIZE];
  char *p = &line_buf[0];
  u64 line_len = 0;
  u32 line_num = 1;
  while (filesystem_read_line(&fd,
                              MATERIAL_FILE_LINE_MAX_SIZE - 1,
                              &p,
                              &line_len)) {
    char *trimmed = kstrtr(line_buf);
    line_len = kstrlen(trimmed);
    // Skip blank lines and comments
    if (!line_len || trimmed[0] == '#') {
      ++line_num;
      continue;
    }
    // Parse the line
    i32 equal_idx = kstridx(trimmed, '=');
    if (equal_idx == -1) {
      KWARN("load_cfg_file :: `%s:%ui` -> skipping line due to '=' char not found",
            path,
            line_num);
      ++line_num;
      continue;
    }
    // Store variable
    char raw_var[MATERIAL_FILE_VAR_MAX_SIZE];
    kzero_memory(raw_var, sizeof(char) * MATERIAL_FILE_VAR_MAX_SIZE);
    kstrsub(raw_var, trimmed, 0, equal_idx);
    char *trimmed_var = kstrtr(raw_var);
    // Store value
    char raw_value[MATERIAL_FILE_VALUE_MAX_SIZE];
    kzero_memory(raw_value, sizeof(char) * MATERIAL_FILE_VALUE_MAX_SIZE);
    kstrsub(raw_value, trimmed, equal_idx + 1, -1);
    char *trimmed_value = kstrtr(raw_value);

    // Process variable against value
    if (kstrcmpi(trimmed_var, "version")) {
      // TODO: handle version
    }
    else if (kstrcmpi(trimmed_var, "name")) {
      kstrncp(out_cfg->name, trimmed_value, MATERIAL_NAME_MAX_LEN);
    }
    else if (kstrcmpi(trimmed_var, "diffuse_map_name")) {
      kstrncp(out_cfg->diffuse_map_name, trimmed_value, TEXTURE_NAME_MAX_LEN);
    }
    else if (kstrcmpi(trimmed_var, "diffuse_color")) {
      if (!str_to_vec4(trimmed_value, &out_cfg->diffuse_color)) {
        KWARN("load_cfg_file :: `%s:%ui` -> `diffuse_color` parse error (using white as fallback)",
              path,
              line_num);
        out_cfg->diffuse_color = vec4_one();  // white
      }
    }

    // Cleanup
    kzero_memory(line_buf, sizeof(char) * MATERIAL_FILE_LINE_MAX_SIZE);
    ++line_num;
  }

  filesystem_close(&fd);
  return true;
}

b8 material_system_initialize(u64 *memory_requirements,
                              void *state,
                              material_system_config config) {
  if (!config.max_material_count) {
    KFATAL("material_system_initialize :: `max_material_count` must be > 0");
    return false;
  }
  u64 struct_requirements = sizeof(material_system_state);
  u64 array_requirements = sizeof(material) * config.max_material_count;
  u64 hash_table_requirements = sizeof(material_ref) * config.max_material_count;
  *memory_requirements = struct_requirements + array_requirements + hash_table_requirements;
  if (!state) return true;

  state_ptr = state;
  state_ptr->config = config;
  void *array_block = (void *) ((u64 *) state + struct_requirements);
  state_ptr->registered_materials = array_block;
  void *hash_table_block = (void *) ((u64 *) array_block + array_requirements);
  hash_table_create(sizeof(material_ref),
                    config.max_material_count,
                    hash_table_block,
                    false,
                    &state_ptr->registered_material_table);

  material_ref invalid_ref = {
    .auto_release = false,
    .handle = INVALID_ID,
    .ref_count = 0
  };
  hash_table_fill(&state_ptr->registered_material_table, &invalid_ref);
  for (u32 i = 0; i < state_ptr->config.max_material_count; ++i) {
    state_ptr->registered_materials[i].id = INVALID_ID;
    state_ptr->registered_materials[i].generation = INVALID_ID;
    state_ptr->registered_materials[i].internal_id = INVALID_ID;
  }

  if (!create_fallback_material(state_ptr)) {
    KFATAL("material_system_initialize :: Fallback material creation failed");
    return false;
  }
  return true;
}

void material_system_shutdown(void *state) {
  material_system_state *s = (material_system_state *) state;
  if (s) {
    for (u32 i = 0; i < s->config.max_material_count; ++i) {
      if (s->registered_materials[i].id != INVALID_ID) {
        destroy_material(&s->registered_materials[i]);
      }
    }
    destroy_material(&s->fallback_material);
  }
  state_ptr = 0;
}

material *material_system_get(const char *name) {
  material_config cfg;
  char *fmt_str = "assets/materials/%s.%s";
  char full_file_path[FILE_PATH_SIZE];

  kstrfmt(full_file_path, fmt_str, name, MATERIAL_FILE_EXTENSION_NAME);
  if (!load_cfg_file(full_file_path, &cfg)) {
    KERROR("material_system_get :: Material file loading failed ('%s')", full_file_path);
    return 0;
  }
  return material_system_get_from_cfg(cfg);
}

material *material_system_get_fallback(void) {
  if (state_ptr) return &state_ptr->fallback_material;
  KFATAL("material_system_get_fallback :: called before material system init");
  return 0;
}

material *material_system_get_from_cfg(material_config cfg) {
  if (kstrcmpi(cfg.name, FALLBACK_MATERIAL_NAME)) return &state_ptr->fallback_material;

  material_ref ref;
  if (!state_ptr || !hash_table_get(&state_ptr->registered_material_table,
                                    cfg.name,
                                    &ref)) {
    KERROR("material_system_get_from_cfg :: get material failed ('%s')", cfg.name);
    return 0;
  }

  if (!ref.ref_count) ref.auto_release = cfg.auto_release;
  ++ref.ref_count;
  if (ref.handle == INVALID_ID) {
    material *m = 0;
    for (u32 i = 0; i < state_ptr->config.max_material_count; ++i) {
      if (state_ptr->registered_materials[i].id == INVALID_ID) {
        ref.handle = i;
        m = &state_ptr->registered_materials[i];
        break;
      }
    }
    if (!m || ref.handle == INVALID_ID) {
      KFATAL("material_system_get_from_cfg :: material system if full (adjust config to allow more materials)");
      return 0;
    }
    if (!load_material(cfg, m)) {
      KERROR("material_system_get_from_cfg :: Material loading failed ('%s')", cfg.name);
      return 0;
    }
    if (m->generation == INVALID_ID) m->generation = 0;
    else ++m->generation;
    m->id = ref.handle;
    KTRACE("Material '%s' created (does not exist yet). `ref_count` -> %i",
           cfg.name,
           ref.ref_count);
  }
  else KTRACE("Material '%s' already exists. `ref_count` -> %i",
              cfg.name,
              ref.ref_count);
  // Update entry
  hash_table_set(&state_ptr->registered_material_table, cfg.name, &ref);
  return &state_ptr->registered_materials[ref.handle];
}

void material_system_release(const char *name) {
  if (kstrcmpi(name, FALLBACK_MATERIAL_NAME)) return;

  material_ref ref;
  if (!state_ptr || !hash_table_get(&state_ptr->registered_material_table,
                                    name,
                                    &ref)) {
    KERROR("material_system_release :: failed to release material '%s'", name);
    return;
  }

  if (!ref.ref_count) {
    KWARN("Tried to release non-existant material ('%s')", name);
    return;
  }
  --ref.ref_count;
  if (!ref.ref_count && ref.auto_release) {
    material *m = &state_ptr->registered_materials[ref.handle];
    destroy_material(m);
    ref.handle = INVALID_ID;
    ref.auto_release = false;
    KTRACE("Material '%s' released (`ref_count` -> 0 && `auto_release` -> true)", name);
  }
  else KTRACE("Material '%s' released (`ref_count` -> %i && `auto_release` -> %s)",
              name,
              ref.ref_count,
              ref.auto_release ? "true" : "false");
  // Update entry
  hash_table_set(&state_ptr->registered_material_table, name, &ref);
}
